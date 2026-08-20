#include "bc2_hw_wallet.h"

#include "bc2_bip39_words.h"
#include "bc2_address.h"
#include "bc2_bip32.h"
#include "bc2_crypto.h"
#include "bc2_network.h"
#include "bc2_sign.h"
#include "bc2_transaction.h"

#include "esp_efuse.h"
#include "esp_hmac.h"
#include "mbedtls/gcm.h"
#include "mbedtls/md.h"

#include <string.h>
#include <stdio.h>

#define BC2_WALLET_KEY_META "wallet_hmac"
#define BC2_WALLET_RECORD_KEY "wallet_seed"
#define BC2_WALLET_RECEIVE_INDEX_KEY "recv_index"
#define BC2_WALLET_RECORD24_KEY "wallet_seed24"
#define BC2_WALLET_META_MAGIC 0x4232574BU
#define BC2_WALLET_RECORD_MAGIC 0x42325753U
#define BC2_WALLET_VERSION 1U
#define BC2_WALLET_ENTROPY_SIZE 16U
#define BC2_WALLET_NONCE_SIZE 12U
#define BC2_WALLET_TAG_SIZE 16U

static const uint8_t k_wrap_context[] = "BC2 wallet seed wrapping key v1";
static const uint8_t k_aad[] = "BC2 wallet seed record v1";
static const uint8_t k_aad24[] = "BC2 wallet 24-word seed record v1";

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t block;
    uint8_t reserved[2];
} bc2_wallet_key_meta_t;

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t backup_confirmed;
    uint8_t reserved[2];
    uint8_t nonce[BC2_WALLET_NONCE_SIZE];
    uint8_t ciphertext[BC2_WALLET_ENTROPY_SIZE];
    uint8_t tag[BC2_WALLET_TAG_SIZE];
} bc2_wallet_record_t;

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t backup_confirmed;
    uint8_t reserved[2];
    uint8_t nonce[BC2_WALLET_NONCE_SIZE];
    uint8_t ciphertext[32];
    uint8_t tag[BC2_WALLET_TAG_SIZE];
} bc2_wallet_record24_t;

static void secure_zero(void *memory, size_t length) {
    volatile uint8_t *cursor = (volatile uint8_t *)memory;
    while (length-- > 0U) *cursor++ = 0U;
}

static bool block_to_hmac_key(uint8_t block, hmac_key_id_t *key_id) {
    if (key_id == NULL) return false;
    switch ((esp_efuse_block_t)block) {
        case EFUSE_BLK_KEY0: *key_id = HMAC_KEY0; return true;
        case EFUSE_BLK_KEY1: *key_id = HMAC_KEY1; return true;
        case EFUSE_BLK_KEY2: *key_id = HMAC_KEY2; return true;
        case EFUSE_BLK_KEY3: *key_id = HMAC_KEY3; return true;
        case EFUSE_BLK_KEY4: *key_id = HMAC_KEY4; return true;
        case EFUSE_BLK_KEY5: *key_id = HMAC_KEY5; return true;
        default: return false;
    }
}

static bool read_key_meta(const bc2_hal_t *hal, bc2_wallet_key_meta_t *meta) {
    size_t size = 0U;
    if (hal == NULL || meta == NULL) return false;
    if (bc2_hal_storage_read(hal, BC2_WALLET_KEY_META, (uint8_t *)meta,
                             sizeof(*meta), &size) != BC2_HAL_OK ||
        size != sizeof(*meta) || meta->magic != BC2_WALLET_META_MAGIC ||
        meta->version != BC2_WALLET_VERSION)
        return false;
    if (esp_efuse_get_key_purpose((esp_efuse_block_t)meta->block) !=
        ESP_EFUSE_KEY_PURPOSE_HMAC_UP)
        return false;
    return true;
}

static bool ensure_hmac_key(const bc2_hal_t *hal, bc2_wallet_key_meta_t *meta) {
    uint8_t key[32];
    esp_efuse_block_t block;
    if (read_key_meta(hal, meta)) return true;

    /* Reserve lower-numbered key blocks for future Secure Boot / flash
     * encryption provisioning where possible. */
    block = EFUSE_BLK_KEY_MAX;
    for (int candidate = (int)EFUSE_BLK_KEY5; candidate >= (int)EFUSE_BLK_KEY0; --candidate) {
        if (esp_efuse_key_block_unused((esp_efuse_block_t)candidate)) {
            block = (esp_efuse_block_t)candidate;
            break;
        }
    }
    if (block == EFUSE_BLK_KEY_MAX) return false;
    if (bc2_hal_random(hal, key, sizeof(key)) != BC2_HAL_OK) return false;
    if (esp_efuse_write_key(block, ESP_EFUSE_KEY_PURPOSE_HMAC_UP,
                            key, sizeof(key)) != ESP_OK) {
        secure_zero(key, sizeof(key));
        return false;
    }
    secure_zero(key, sizeof(key));

    memset(meta, 0, sizeof(*meta));
    meta->magic = BC2_WALLET_META_MAGIC;
    meta->version = BC2_WALLET_VERSION;
    meta->block = (uint8_t)block;
    return bc2_hal_storage_write(hal, BC2_WALLET_KEY_META,
                                 (const uint8_t *)meta,
                                 sizeof(*meta)) == BC2_HAL_OK;
}

static bool derive_wrap_key(const bc2_wallet_key_meta_t *meta, uint8_t key[32]) {
    hmac_key_id_t key_id;
    if (meta == NULL || key == NULL || !block_to_hmac_key(meta->block, &key_id))
        return false;
    return esp_hmac_calculate(key_id, k_wrap_context,
                              sizeof(k_wrap_context) - 1U, key) == ESP_OK;
}

static bool entropy_to_words(const uint8_t entropy[BC2_WALLET_ENTROPY_SIZE],
                             char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]) {
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint8_t hash[32];
    uint16_t indexes[BC2_HW_WALLET_WORD_COUNT] = {0};
    size_t bit;
    if (sha256 == NULL || mbedtls_md(sha256, entropy, BC2_WALLET_ENTROPY_SIZE, hash) != 0)
        return false;

    for (bit = 0U; bit < 132U; ++bit) {
        uint8_t value;
        if (bit < 128U)
            value = (uint8_t)((entropy[bit / 8U] >> (7U - (bit % 8U))) & 1U);
        else
            value = (uint8_t)((hash[0] >> (7U - (bit - 128U))) & 1U);
        indexes[bit / 11U] = (uint16_t)((indexes[bit / 11U] << 1U) | value);
    }

    for (bit = 0U; bit < BC2_HW_WALLET_WORD_COUNT; ++bit) {
        size_t length = strlen(bc2_bip39_english_words[indexes[bit]]);
        if (length >= BC2_HW_WALLET_WORD_SIZE) {
            secure_zero(hash, sizeof(hash));
            return false;
        }
        memcpy(words[bit], bc2_bip39_english_words[indexes[bit]], length + 1U);
    }
    secure_zero(hash, sizeof(hash));
    secure_zero(indexes, sizeof(indexes));
    return true;
}

static bool read_record(const bc2_hal_t *hal, bc2_wallet_record_t *record) {
    size_t size = 0U;
    if (hal == NULL || record == NULL) return false;
    return bc2_hal_storage_read(hal, BC2_WALLET_RECORD_KEY,
                                (uint8_t *)record, sizeof(*record), &size) == BC2_HAL_OK &&
           size == sizeof(*record) && record->magic == BC2_WALLET_RECORD_MAGIC &&
           record->version == BC2_WALLET_VERSION;
}

static bool decrypt_entropy(const bc2_hal_t *hal, uint8_t entropy[BC2_WALLET_ENTROPY_SIZE]) {
    bc2_wallet_key_meta_t meta;
    bc2_wallet_record_t record;
    uint8_t wrap_key[32];
    mbedtls_gcm_context gcm;
    int result;
    if (!read_key_meta(hal, &meta) || !read_record(hal, &record) ||
        !derive_wrap_key(&meta, wrap_key))
        return false;

    mbedtls_gcm_init(&gcm);
    result = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, wrap_key, 256U);
    if (result == 0) {
        result = mbedtls_gcm_auth_decrypt(&gcm, BC2_WALLET_ENTROPY_SIZE,
                                          record.nonce, sizeof(record.nonce),
                                          k_aad, sizeof(k_aad) - 1U,
                                          record.tag, sizeof(record.tag),
                                          record.ciphertext, entropy);
    }
    mbedtls_gcm_free(&gcm);
    secure_zero(wrap_key, sizeof(wrap_key));
    secure_zero(&record, sizeof(record));
    secure_zero(&meta, sizeof(meta));
    if (result != 0) secure_zero(entropy, BC2_WALLET_ENTROPY_SIZE);
    return result == 0;
}

bc2_hw_wallet_status_t bc2_hw_wallet_status(const bc2_hal_t *hal) {
    bc2_wallet_record_t record;
    size_t size = 0U;
    bc2_hal_result_t result;
    if (hal == NULL) return BC2_HW_WALLET_ERROR;
    memset(&record, 0, sizeof(record));
    result = bc2_hal_storage_read(hal, BC2_WALLET_RECORD_KEY,
                                  (uint8_t *)&record, sizeof(record), &size);
    if (result == BC2_HAL_ERROR_NOT_FOUND) {
        bc2_wallet_record24_t record24;
        size_t size24 = 0U;
        memset(&record24, 0, sizeof(record24));
        result = bc2_hal_storage_read(hal, BC2_WALLET_RECORD24_KEY,
                                      (uint8_t *)&record24, sizeof(record24), &size24);
        if (result == BC2_HAL_ERROR_NOT_FOUND) return BC2_HW_WALLET_NONE;
        if (result != BC2_HAL_OK || size24 != sizeof(record24) ||
            record24.magic != BC2_WALLET_RECORD_MAGIC ||
            record24.version != BC2_WALLET_VERSION) {
            secure_zero(&record24, sizeof(record24));
            return BC2_HW_WALLET_ERROR;
        }
        const bc2_hw_wallet_status_t status24 = record24.backup_confirmed
            ? BC2_HW_WALLET_READY : BC2_HW_WALLET_BACKUP_PENDING;
        secure_zero(&record24, sizeof(record24));
        return status24;
    }
    if (result != BC2_HAL_OK || size != sizeof(record) ||
        record.magic != BC2_WALLET_RECORD_MAGIC ||
        record.version != BC2_WALLET_VERSION) {
        secure_zero(&record, sizeof(record));
        return BC2_HW_WALLET_ERROR;
    }
    const bc2_hw_wallet_status_t status = record.backup_confirmed
        ? BC2_HW_WALLET_READY : BC2_HW_WALLET_BACKUP_PENDING;
    secure_zero(&record, sizeof(record));
    return status;
}

bool bc2_hw_wallet_create(const bc2_hal_t *hal,
                          char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]) {
    bc2_wallet_key_meta_t meta;
    bc2_wallet_record_t record;
    uint8_t entropy[BC2_WALLET_ENTROPY_SIZE];
    uint8_t wrap_key[32];
    mbedtls_gcm_context gcm;
    int result = -1;

    if (hal == NULL || words == NULL || bc2_hw_wallet_status(hal) != BC2_HW_WALLET_NONE)
        return false;
    if (!ensure_hmac_key(hal, &meta) ||
        bc2_hal_random(hal, entropy, sizeof(entropy)) != BC2_HAL_OK ||
        !derive_wrap_key(&meta, wrap_key) ||
        !entropy_to_words(entropy, words))
        goto cleanup;

    memset(&record, 0, sizeof(record));
    record.magic = BC2_WALLET_RECORD_MAGIC;
    record.version = BC2_WALLET_VERSION;
    if (bc2_hal_random(hal, record.nonce, sizeof(record.nonce)) != BC2_HAL_OK)
        goto cleanup;

    mbedtls_gcm_init(&gcm);
    result = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, wrap_key, 256U);
    if (result == 0) {
        result = mbedtls_gcm_crypt_and_tag(&gcm, MBEDTLS_GCM_ENCRYPT,
                                           sizeof(entropy), record.nonce,
                                           sizeof(record.nonce), k_aad,
                                           sizeof(k_aad) - 1U, entropy,
                                           record.ciphertext, sizeof(record.tag),
                                           record.tag);
    }
    mbedtls_gcm_free(&gcm);
    if (result != 0 ||
        bc2_hal_storage_write(hal, BC2_WALLET_RECORD_KEY,
                              (const uint8_t *)&record,
                              sizeof(record)) != BC2_HAL_OK)
        goto cleanup;

    result = 0;
cleanup:
    secure_zero(entropy, sizeof(entropy));
    secure_zero(wrap_key, sizeof(wrap_key));
    secure_zero(&record, sizeof(record));
    secure_zero(&meta, sizeof(meta));
    if (result != 0) bc2_hw_wallet_clear_words(words);
    return result == 0;
}

bool bc2_hw_wallet_load_words(const bc2_hal_t *hal,
                              char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]) {
    uint8_t entropy[BC2_WALLET_ENTROPY_SIZE];
    bool ok;
    if (hal == NULL || words == NULL ||
        bc2_hw_wallet_status(hal) != BC2_HW_WALLET_BACKUP_PENDING)
        return false;
    ok = decrypt_entropy(hal, entropy) && entropy_to_words(entropy, words);
    secure_zero(entropy, sizeof(entropy));
    if (!ok) bc2_hw_wallet_clear_words(words);
    return ok;
}

bool bc2_hw_wallet_confirm_backup(const bc2_hal_t *hal) {
    bc2_wallet_record_t record;
    bool ok;
    if (!read_record(hal, &record) || record.backup_confirmed) return false;
    record.backup_confirmed = 1U;
    ok = bc2_hal_storage_write(hal, BC2_WALLET_RECORD_KEY,
                               (const uint8_t *)&record,
                               sizeof(record)) == BC2_HAL_OK;
    secure_zero(&record, sizeof(record));
    return ok;
}


static bool entropy_to_mnemonic_any(const uint8_t *entropy, size_t entropy_size,
                                    char *mnemonic, size_t capacity) {
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint8_t hash[32];
    uint16_t indexes[BC2_HW_WALLET_MAX_WORD_COUNT] = {0};
    size_t word_count;
    size_t total_bits;
    size_t used = 0U;
    if (entropy == NULL || mnemonic == NULL || capacity == 0U ||
        (entropy_size != 16U && entropy_size != 32U) || sha256 == NULL)
        return false;
    word_count = entropy_size == 16U ? 12U : 24U;
    if (mbedtls_md(sha256, entropy, entropy_size, hash) != 0) return false;
    total_bits = entropy_size * 8U + entropy_size / 4U;
    for (size_t bit = 0U; bit < total_bits; ++bit) {
        uint8_t value;
        if (bit < entropy_size * 8U)
            value = (uint8_t)((entropy[bit / 8U] >> (7U - (bit % 8U))) & 1U);
        else {
            const size_t cbit = bit - entropy_size * 8U;
            value = (uint8_t)((hash[cbit / 8U] >> (7U - (cbit % 8U))) & 1U);
        }
        indexes[bit / 11U] = (uint16_t)((indexes[bit / 11U] << 1U) | value);
    }
    for (size_t word = 0U; word < word_count; ++word) {
        const char *text = bc2_bip39_english_words[indexes[word]];
        const size_t length = strlen(text);
        const size_t separator = word == 0U ? 0U : 1U;
        if (used + separator + length + 1U > capacity) goto fail;
        if (separator) mnemonic[used++] = ' ';
        memcpy(mnemonic + used, text, length);
        used += length;
    }
    mnemonic[used] = '\0';
    secure_zero(hash, sizeof(hash));
    secure_zero(indexes, sizeof(indexes));
    return true;
fail:
    secure_zero(hash, sizeof(hash));
    secure_zero(indexes, sizeof(indexes));
    secure_zero(mnemonic, capacity);
    return false;
}

static bool decrypt_entropy_any(const bc2_hal_t *hal, uint8_t entropy[32], size_t *entropy_size) {
    bc2_wallet_key_meta_t meta;
    uint8_t wrap_key[32];
    mbedtls_gcm_context gcm;
    size_t size = 0U;
    int result = -1;
    if (hal == NULL || entropy == NULL || entropy_size == NULL ||
        !read_key_meta(hal, &meta) || !derive_wrap_key(&meta, wrap_key))
        return false;
    bc2_wallet_record_t record;
    memset(&record, 0, sizeof(record));
    bc2_hal_result_t read = bc2_hal_storage_read(hal, BC2_WALLET_RECORD_KEY,
                                                (uint8_t *)&record, sizeof(record), &size);
    mbedtls_gcm_init(&gcm);
    if (read == BC2_HAL_OK && size == sizeof(record) &&
        record.magic == BC2_WALLET_RECORD_MAGIC && record.version == BC2_WALLET_VERSION) {
        result = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, wrap_key, 256U);
        if (result == 0)
            result = mbedtls_gcm_auth_decrypt(&gcm, 16U, record.nonce, sizeof(record.nonce),
                                              k_aad, sizeof(k_aad)-1U, record.tag, sizeof(record.tag),
                                              record.ciphertext, entropy);
        if (result == 0) *entropy_size = 16U;
    } else {
        bc2_wallet_record24_t record24;
        size_t size24 = 0U;
        memset(&record24, 0, sizeof(record24));
        read = bc2_hal_storage_read(hal, BC2_WALLET_RECORD24_KEY,
                                    (uint8_t *)&record24, sizeof(record24), &size24);
        if (read == BC2_HAL_OK && size24 == sizeof(record24) &&
            record24.magic == BC2_WALLET_RECORD_MAGIC && record24.version == BC2_WALLET_VERSION) {
            result = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, wrap_key, 256U);
            if (result == 0)
                result = mbedtls_gcm_auth_decrypt(&gcm, 32U, record24.nonce, sizeof(record24.nonce),
                                                  k_aad24, sizeof(k_aad24)-1U, record24.tag, sizeof(record24.tag),
                                                  record24.ciphertext, entropy);
            if (result == 0) *entropy_size = 32U;
        }
        secure_zero(&record24, sizeof(record24));
    }
    mbedtls_gcm_free(&gcm);
    secure_zero(&record, sizeof(record));
    secure_zero(wrap_key, sizeof(wrap_key));
    secure_zero(&meta, sizeof(meta));
    if (result != 0) secure_zero(entropy, 32U);
    return result == 0;
}


bool bc2_hw_wallet_id(const bc2_hal_t *hal,
                      uint8_t wallet_id[BC2_HW_WALLET_ID_SIZE]) {
    static const uint8_t domain[] = "BC2 wallet id v1";
    uint8_t entropy[32] = {0};
    size_t entropy_size = 0U;
    uint8_t seed[64] = {0};
    uint8_t public_key[33] = {0};
    uint8_t digest[32] = {0};
    uint8_t material[(sizeof(domain) - 1U) + sizeof(public_key)];
    bc2_xprv master = {0};
    bc2_xprv account = {0};
    char mnemonic[256] = {0};
    char path[64] = {0};
    char salt[] = "mnemonic";
    const bc2_network *network = bc2_network_mainnet();
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    bool ok = false;

    if (hal == NULL || wallet_id == NULL || network == NULL || sha256 == NULL ||
        bc2_hw_wallet_status(hal) != BC2_HW_WALLET_READY)
        return false;

    memset(wallet_id, 0, BC2_HW_WALLET_ID_SIZE);
    if (!decrypt_entropy_any(hal, entropy, &entropy_size) ||
        !entropy_to_mnemonic_any(entropy, entropy_size, mnemonic, sizeof(mnemonic)) ||
        !bc2_pbkdf2_hmac_sha512((const uint8_t *)mnemonic, strlen(mnemonic),
                                (const uint8_t *)salt, strlen(salt),
                                2048U, seed, sizeof(seed)))
        goto cleanup;

    const int written = snprintf(path, sizeof(path), "m/84'/%u'/0'",
                                 (unsigned int)network->coin_type);
    if (written < 0 || (size_t)written >= sizeof(path) ||
        !bc2_bip32_master(seed, sizeof(seed), &master) ||
        !bc2_bip32_derive_path(&master, path, &account) ||
        !bc2_secp256k1_public(account.key, public_key))
        goto cleanup;

    memcpy(material, domain, sizeof(domain) - 1U);
    memcpy(material + sizeof(domain) - 1U, public_key, sizeof(public_key));
    if (mbedtls_md(sha256, material, sizeof(material), digest) != 0)
        goto cleanup;

    memcpy(wallet_id, digest, BC2_HW_WALLET_ID_SIZE);
    ok = true;

cleanup:
    secure_zero(entropy, sizeof(entropy));
    secure_zero(seed, sizeof(seed));
    secure_zero(public_key, sizeof(public_key));
    secure_zero(digest, sizeof(digest));
    secure_zero(material, sizeof(material));
    secure_zero(&master, sizeof(master));
    secure_zero(&account, sizeof(account));
    secure_zero(mnemonic, sizeof(mnemonic));
    secure_zero(path, sizeof(path));
    if (!ok) memset(wallet_id, 0, BC2_HW_WALLET_ID_SIZE);
    return ok;
}

bool bc2_hw_wallet_receive_address(const bc2_hal_t *hal, uint32_t index,
                                   char *address, size_t address_capacity) {
    uint8_t entropy[32];
    size_t entropy_size = 0U;
    uint8_t seed[64];
    uint8_t public_key[33];
    bc2_xprv master;
    bc2_xprv node;
    char mnemonic[256];
    char salt[] = "mnemonic";
    char path[96];
    const bc2_network *network = bc2_network_mainnet();
    bool ok = false;

    if (hal == NULL || address == NULL || address_capacity == 0U ||
        index > 0x7fffffffU || network == NULL ||
        bc2_hw_wallet_status(hal) != BC2_HW_WALLET_READY)
        return false;

    memset(entropy, 0, sizeof(entropy));
    memset(seed, 0, sizeof(seed));
    memset(public_key, 0, sizeof(public_key));
    memset(&master, 0, sizeof(master));
    memset(&node, 0, sizeof(node));
    memset(mnemonic, 0, sizeof(mnemonic));
    address[0] = '\0';

    if (!decrypt_entropy_any(hal, entropy, &entropy_size) ||
        !entropy_to_mnemonic_any(entropy, entropy_size, mnemonic, sizeof(mnemonic)) ||
        !bc2_pbkdf2_hmac_sha512((const uint8_t *)mnemonic, strlen(mnemonic),
                                 (const uint8_t *)salt, strlen(salt),
                                 2048U, seed, sizeof(seed)))
        goto cleanup;

    const int written = snprintf(path, sizeof(path), "m/84'/%u'/0'/0/%u",
                                 (unsigned int)network->coin_type,
                                 (unsigned int)index);
    if (written < 0 || (size_t)written >= sizeof(path) ||
        !bc2_bip32_master(seed, sizeof(seed), &master) ||
        !bc2_bip32_derive_path(&master, path, &node) ||
        !bc2_secp256k1_public(node.key, public_key) ||
        !bc2_address_p2wpkh(public_key, network->bech32_hrp,
                            address, address_capacity))
        goto cleanup;

    ok = true;

cleanup:
    secure_zero(entropy, sizeof(entropy));
    secure_zero(seed, sizeof(seed));
    secure_zero(public_key, sizeof(public_key));
    secure_zero(&master, sizeof(master));
    secure_zero(&node, sizeof(node));
    secure_zero(mnemonic, sizeof(mnemonic));
    secure_zero(path, sizeof(path));
    if (!ok) address[0] = '\0';
    return ok;
}


bool bc2_hw_wallet_sign_single_p2wpkh(
    const bc2_hal_t *hal,
    const char *input_address,
    const uint8_t prev_txid_le[32],
    uint32_t prev_output_index,
    uint64_t input_amount,
    uint32_t sequence,
    const char *recipient_address,
    uint64_t recipient_amount,
    uint64_t change_amount,
    uint32_t lock_time,
    uint8_t public_key[33],
    uint8_t *signature,
    size_t signature_capacity,
    size_t *signature_length)
{
    uint8_t entropy[32] = {0};
    uint8_t seed[64] = {0};
    uint8_t pubkey_hash[20] = {0};
    uint8_t digest[32] = {0};
    uint8_t recipient_script[128] = {0};
    uint8_t change_script[128] = {0};

    size_t entropy_size = 0U;
    size_t recipient_script_length = 0U;
    size_t change_script_length = 0U;

    bc2_xprv master = {0};
    bc2_xprv node = {0};

    char mnemonic[256] = {0};
    char path[96] = {0};
    char derived_address[96] = {0};
    char salt[] = "mnemonic";

    uint32_t receive_count = 0U;
    const bc2_network *network = bc2_network_mainnet();
    bool ok = false;

    if (hal == NULL ||
        input_address == NULL ||
        prev_txid_le == NULL ||
        recipient_address == NULL ||
        recipient_amount == 0U ||
        public_key == NULL ||
        signature == NULL ||
        signature_length == NULL ||
        network == NULL ||
        bc2_hw_wallet_status(hal) != BC2_HW_WALLET_READY) {
        return false;
    }

    *signature_length = 0U;
    memset(public_key, 0, 33U);

    if (!bc2_hw_wallet_receive_index(hal, &receive_count) ||
        receive_count == 0U ||
        !decrypt_entropy_any(hal, entropy, &entropy_size) ||
        !entropy_to_mnemonic_any(
            entropy,
            entropy_size,
            mnemonic,
            sizeof(mnemonic)) ||
        !bc2_pbkdf2_hmac_sha512(
            (const uint8_t *)mnemonic,
            strlen(mnemonic),
            (const uint8_t *)salt,
            strlen(salt),
            2048U,
            seed,
            sizeof(seed)) ||
        !bc2_bip32_master(seed, sizeof(seed), &master)) {
        goto cleanup;
    }

    for (uint32_t index = 0U; index < receive_count; ++index) {
        secure_zero(&node, sizeof(node));
        memset(derived_address, 0, sizeof(derived_address));
        memset(public_key, 0, 33U);

        const int written = snprintf(
            path,
            sizeof(path),
            "m/84'/%u'/0'/0/%u",
            (unsigned int)network->coin_type,
            (unsigned int)index);

        if (written < 0 ||
            (size_t)written >= sizeof(path) ||
            !bc2_bip32_derive_path(&master, path, &node) ||
            !bc2_secp256k1_public(node.key, public_key) ||
            !bc2_address_p2wpkh(
                public_key,
                network->bech32_hrp,
                derived_address,
                sizeof(derived_address))) {
            goto cleanup;
        }

        if (strcmp(derived_address, input_address) != 0) {
            continue;
        }

        if (!bc2_hash160(public_key, 33U, pubkey_hash) ||
            bc2_address_to_script(
                recipient_address,
                network,
                recipient_script,
                sizeof(recipient_script),
                &recipient_script_length) != BC2_TX_OK ||
            bc2_address_to_script(
                input_address,
                network,
                change_script,
                sizeof(change_script),
                &change_script_length) != BC2_TX_OK ||
            change_script_length != 22U ||
            change_script[0] != 0x00U ||
            change_script[1] != 0x14U ||
            !bc2_p2wpkh_sighash_all_single(
                prev_txid_le,
                prev_output_index,
                input_amount,
                pubkey_hash,
                sequence,
                recipient_script,
                recipient_script_length,
                recipient_amount,
                change_amount > 0U ? change_script : NULL,
                change_amount > 0U ? change_script_length : 0U,
                change_amount,
                lock_time,
                digest) ||
            !bc2_ecdsa_sign_der(
                node.key,
                digest,
                signature,
                signature_capacity,
                signature_length)) {
            goto cleanup;
        }

        ok = true;
        break;
    }

cleanup:
    secure_zero(entropy, sizeof(entropy));
    secure_zero(seed, sizeof(seed));
    secure_zero(&master, sizeof(master));
    secure_zero(&node, sizeof(node));
    secure_zero(mnemonic, sizeof(mnemonic));
    secure_zero(path, sizeof(path));
    secure_zero(derived_address, sizeof(derived_address));
    secure_zero(pubkey_hash, sizeof(pubkey_hash));
    secure_zero(digest, sizeof(digest));
    secure_zero(recipient_script, sizeof(recipient_script));
    secure_zero(change_script, sizeof(change_script));

    if (!ok) {
        memset(public_key, 0, 33U);

        if (signature_capacity > 0U) {
            memset(signature, 0, signature_capacity);
        }

        *signature_length = 0U;
    }

    return ok;
}

bool bc2_hw_wallet_receive_index(const bc2_hal_t *hal, uint32_t *index) {
    uint8_t data[4];
    size_t size = 0U;
    bc2_hal_result_t result;

    if (hal == NULL || index == NULL) return false;
    result = bc2_hal_storage_read(hal, BC2_WALLET_RECEIVE_INDEX_KEY,
                                  data, sizeof(data), &size);
    if (result == BC2_HAL_ERROR_NOT_FOUND) {
        *index = 0U;
        return true;
    }
    if (result != BC2_HAL_OK || size != sizeof(data)) return false;

    *index = (uint32_t)data[0] |
             ((uint32_t)data[1] << 8U) |
             ((uint32_t)data[2] << 16U) |
             ((uint32_t)data[3] << 24U);
    return *index <= 0x7fffffffU;
}

bool bc2_hw_wallet_commit_receive_index(const bc2_hal_t *hal, uint32_t index) {
    uint32_t current;
    uint32_t next;
    uint8_t data[4];

    if (hal == NULL || index >= 0x7fffffffU ||
        !bc2_hw_wallet_receive_index(hal, &current) || current != index)
        return false;

    next = index + 1U;
    data[0] = (uint8_t)next;
    data[1] = (uint8_t)(next >> 8U);
    data[2] = (uint8_t)(next >> 16U);
    data[3] = (uint8_t)(next >> 24U);
    return bc2_hal_storage_write(hal, BC2_WALLET_RECEIVE_INDEX_KEY,
                                 data, sizeof(data)) == BC2_HAL_OK;
}

bool bc2_hw_wallet_factory_reset(const bc2_hal_t *hal) {
    if (hal == NULL) return false;
    bc2_hal_result_t a = bc2_hal_storage_remove(hal, BC2_WALLET_RECORD_KEY);
    bc2_hal_result_t b = bc2_hal_storage_remove(hal, BC2_WALLET_RECORD24_KEY);
    bc2_hal_result_t c = bc2_hal_storage_remove(hal, BC2_WALLET_RECEIVE_INDEX_KEY);
    return (a == BC2_HAL_OK || a == BC2_HAL_ERROR_NOT_FOUND) &&
           (b == BC2_HAL_OK || b == BC2_HAL_ERROR_NOT_FOUND) &&
           (c == BC2_HAL_OK || c == BC2_HAL_ERROR_NOT_FOUND);
}

bool bc2_hw_wallet_validate_indexes(const uint16_t *indexes, size_t word_count) {
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint8_t entropy[32] = {0};
    uint8_t hash[32] = {0};
    size_t entropy_bits, checksum_bits, entropy_size;
    bool ok = false;
    if (indexes == NULL || (word_count != 12U && word_count != 24U) || sha256 == NULL)
        return false;
    entropy_bits = word_count == 12U ? 128U : 256U;
    checksum_bits = word_count == 12U ? 4U : 8U;
    entropy_size = entropy_bits / 8U;
    for (size_t i = 0U; i < word_count; ++i)
        if (indexes[i] >= 2048U) goto cleanup;
    for (size_t bit=0; bit<entropy_bits; ++bit) {
        size_t wi=bit/11U, off=bit%11U;
        uint8_t v=(uint8_t)((indexes[wi]>>(10U-off))&1U);
        entropy[bit/8U] |= (uint8_t)(v << (7U-(bit%8U)));
    }
    if (mbedtls_md(sha256, entropy, entropy_size, hash) != 0) goto cleanup;
    for (size_t bit=0; bit<checksum_bits; ++bit) {
        size_t pos=entropy_bits+bit, wi=pos/11U, off=pos%11U;
        uint8_t got=(uint8_t)((indexes[wi]>>(10U-off))&1U);
        uint8_t exp=(uint8_t)((hash[0]>>(7U-bit))&1U);
        if (got != exp) goto cleanup;
    }
    ok = true;
cleanup:
    secure_zero(entropy,sizeof(entropy)); secure_zero(hash,sizeof(hash));
    return ok;
}

bool bc2_hw_wallet_restore_indexes(const bc2_hal_t *hal, const uint16_t *indexes, size_t word_count) {
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    bc2_wallet_key_meta_t meta;
    uint8_t entropy[32] = {0};
    uint8_t hash[32];
    uint8_t wrap_key[32];
    size_t entropy_bits, checksum_bits, entropy_size;
    bool ok = false;
    if (hal == NULL || indexes == NULL || (word_count != 12U && word_count != 24U) || sha256 == NULL ||
        !bc2_hw_wallet_validate_indexes(indexes, word_count))
        return false;
    entropy_bits = word_count == 12U ? 128U : 256U;
    checksum_bits = word_count == 12U ? 4U : 8U;
    entropy_size = entropy_bits / 8U;
    for (size_t bit=0; bit<entropy_bits; ++bit) {
        size_t wi=bit/11U, off=bit%11U;
        uint8_t v=(uint8_t)((indexes[wi]>>(10U-off))&1U);
        entropy[bit/8U] |= (uint8_t)(v << (7U-(bit%8U)));
    }
    if (mbedtls_md(sha256, entropy, entropy_size, hash) != 0) goto cleanup;
    for (size_t bit=0; bit<checksum_bits; ++bit) {
        size_t pos=entropy_bits+bit, wi=pos/11U, off=pos%11U;
        uint8_t got=(uint8_t)((indexes[wi]>>(10U-off))&1U);
        uint8_t exp=(uint8_t)((hash[0]>>(7U-bit))&1U);
        if (got != exp) goto cleanup;
    }
    if (bc2_hw_wallet_status(hal) != BC2_HW_WALLET_NONE || !ensure_hmac_key(hal,&meta) || !derive_wrap_key(&meta,wrap_key))
        goto cleanup;
    if (word_count == 12U) {
        bc2_wallet_record_t record;
        mbedtls_gcm_context gcm;
        memset(&record,0,sizeof(record)); record.magic=BC2_WALLET_RECORD_MAGIC; record.version=BC2_WALLET_VERSION; record.backup_confirmed=1U;
        if (bc2_hal_random(hal,record.nonce,sizeof(record.nonce)) != BC2_HAL_OK) goto cleanup;
        mbedtls_gcm_init(&gcm);
        int r=mbedtls_gcm_setkey(&gcm,MBEDTLS_CIPHER_ID_AES,wrap_key,256U);
        if (r==0) r=mbedtls_gcm_crypt_and_tag(&gcm,MBEDTLS_GCM_ENCRYPT,16U,record.nonce,sizeof(record.nonce),k_aad,sizeof(k_aad)-1U,entropy,record.ciphertext,sizeof(record.tag),record.tag);
        mbedtls_gcm_free(&gcm);
        if (r==0 && bc2_hal_storage_write(hal,BC2_WALLET_RECORD_KEY,(const uint8_t*)&record,sizeof(record))==BC2_HAL_OK) ok=true;
        secure_zero(&record,sizeof(record));
    } else {
        bc2_wallet_record24_t record;
        mbedtls_gcm_context gcm;
        memset(&record,0,sizeof(record)); record.magic=BC2_WALLET_RECORD_MAGIC; record.version=BC2_WALLET_VERSION; record.backup_confirmed=1U;
        if (bc2_hal_random(hal,record.nonce,sizeof(record.nonce)) != BC2_HAL_OK) goto cleanup;
        mbedtls_gcm_init(&gcm);
        int r=mbedtls_gcm_setkey(&gcm,MBEDTLS_CIPHER_ID_AES,wrap_key,256U);
        if (r==0) r=mbedtls_gcm_crypt_and_tag(&gcm,MBEDTLS_GCM_ENCRYPT,32U,record.nonce,sizeof(record.nonce),k_aad24,sizeof(k_aad24)-1U,entropy,record.ciphertext,sizeof(record.tag),record.tag);
        mbedtls_gcm_free(&gcm);
        if (r==0 && bc2_hal_storage_write(hal,BC2_WALLET_RECORD24_KEY,(const uint8_t*)&record,sizeof(record))==BC2_HAL_OK) ok=true;
        secure_zero(&record,sizeof(record));
    }
cleanup:
    secure_zero(entropy,sizeof(entropy)); secure_zero(hash,sizeof(hash)); secure_zero(wrap_key,sizeof(wrap_key)); secure_zero(&meta,sizeof(meta));
    return ok;
}

void bc2_hw_wallet_clear_words(
    char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]) {
    if (words != NULL) secure_zero(words,
        BC2_HW_WALLET_WORD_COUNT * BC2_HW_WALLET_WORD_SIZE);
}
