#include "bc2_pin_security.h"

#include <stddef.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "mbedtls/md.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#else
#include <openssl/evp.h>
#endif

#define BC2_PIN_RECORD_KEY "device_pin"
#define BC2_PIN_MAGIC 0x42324350UL
#define BC2_PIN_RECORD_VERSION 2U
#define BC2_PIN_KDF_ROUNDS 100000U
#define BC2_PIN_SALT_SIZE 16U
#define BC2_PIN_HASH_SIZE 32U
#define BC2_PIN_KDF_YIELD_INTERVAL 1024U

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t failures;
    uint8_t reserved[2];
    uint8_t salt[BC2_PIN_SALT_SIZE];
    uint8_t hash[BC2_PIN_HASH_SIZE];
} bc2_pin_record_t;

static void secure_clear(void *data, size_t size) {
    volatile uint8_t *cursor = (volatile uint8_t *)data;
    while (size-- > 0U) *cursor++ = 0U;
}

static bool valid_pin(const char *pin) {
    size_t index;
    if (pin == NULL || strlen(pin) != BC2_SECURITY_PIN_LENGTH) return false;
    for (index = 0U; index < BC2_SECURITY_PIN_LENGTH; ++index)
        if (pin[index] < '0' || pin[index] > '9') return false;
    return true;
}

static bool derive(const char *pin, const uint8_t salt[BC2_PIN_SALT_SIZE],
                   uint8_t output[BC2_PIN_HASH_SIZE]) {
#ifdef ESP_PLATFORM
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint8_t input[BC2_PIN_SALT_SIZE + 4U];
    uint8_t digest[BC2_PIN_HASH_SIZE];
    uint8_t next_digest[BC2_PIN_HASH_SIZE];
    uint32_t round;
    size_t index;
    int result;

    if (md == NULL) return false;
    memcpy(input, salt, BC2_PIN_SALT_SIZE);
    input[BC2_PIN_SALT_SIZE] = 0U;
    input[BC2_PIN_SALT_SIZE + 1U] = 0U;
    input[BC2_PIN_SALT_SIZE + 2U] = 0U;
    input[BC2_PIN_SALT_SIZE + 3U] = 1U;

    result = mbedtls_md_hmac(md, (const unsigned char *)pin,
                             BC2_SECURITY_PIN_LENGTH, input, sizeof(input), digest);
    if (result != 0) {
        secure_clear(input, sizeof(input));
        secure_clear(digest, sizeof(digest));
        return false;
    }
    memcpy(output, digest, BC2_PIN_HASH_SIZE);

    for (round = 1U; round < BC2_PIN_KDF_ROUNDS; ++round) {
        result = mbedtls_md_hmac(md, (const unsigned char *)pin,
                                 BC2_SECURITY_PIN_LENGTH, digest, sizeof(digest),
                                 next_digest);
        if (result != 0) {
            secure_clear(input, sizeof(input));
            secure_clear(digest, sizeof(digest));
            secure_clear(next_digest, sizeof(next_digest));
            secure_clear(output, BC2_PIN_HASH_SIZE);
            return false;
        }
        for (index = 0U; index < BC2_PIN_HASH_SIZE; ++index)
            output[index] ^= next_digest[index];
        memcpy(digest, next_digest, sizeof(digest));
        if ((round % BC2_PIN_KDF_YIELD_INTERVAL) == 0U)
            vTaskDelay(1U);
    }
    secure_clear(input, sizeof(input));
    secure_clear(digest, sizeof(digest));
    secure_clear(next_digest, sizeof(next_digest));
    return true;
#else
    return PKCS5_PBKDF2_HMAC(pin, (int)BC2_SECURITY_PIN_LENGTH,
                             salt, (int)BC2_PIN_SALT_SIZE,
                             (int)BC2_PIN_KDF_ROUNDS, EVP_sha256(),
                             (int)BC2_PIN_HASH_SIZE, output) == 1;
#endif
}

static uint64_t delay_ms(uint8_t failures) {
    static const uint64_t delays[] = {0U, 0U, 0U, 5000U, 15000U, 30000U,
                                     60000U, 300000U, 900000U, 1800000U, 3600000U};
    if (failures >= BC2_SECURITY_PIN_MAX_FAILURES) return delays[10];
    return delays[failures];
}

static bc2_pin_security_result_t read_record(const bc2_hal_t *hal, bc2_pin_record_t *record) {
    size_t size = 0U;
    const bc2_hal_result_t result = bc2_hal_storage_read(
        hal, BC2_PIN_RECORD_KEY, (uint8_t *)record, sizeof(*record), &size);
    if (result == BC2_HAL_ERROR_NOT_FOUND) return BC2_PIN_SECURITY_NOT_CONFIGURED;
    if (result != BC2_HAL_OK || size != sizeof(*record) ||
        record->magic != BC2_PIN_MAGIC)
        return BC2_PIN_SECURITY_ERROR;
    if (record->version == 1U) {
        /* Legacy v0.29.x six-digit PIN. No seed was persisted by that build.
         * Remove only the obsolete PIN record and force clean 4-digit setup. */
        secure_clear(record, sizeof(*record));
        return bc2_hal_storage_remove(hal, BC2_PIN_RECORD_KEY) == BC2_HAL_OK
            ? BC2_PIN_SECURITY_NOT_CONFIGURED
            : BC2_PIN_SECURITY_ERROR;
    }
    if (record->version != BC2_PIN_RECORD_VERSION)
        return BC2_PIN_SECURITY_ERROR;
    return BC2_PIN_SECURITY_OK;
}

bc2_pin_security_result_t bc2_pin_security_load(bc2_pin_security_t *security,
                                                const bc2_hal_t *hal,
                                                uint64_t now_ms) {
    bc2_pin_record_t record;
    bc2_pin_security_result_t result;
    if (security == NULL || hal == NULL) return BC2_PIN_SECURITY_ERROR;
    memset(security, 0, sizeof(*security));
    result = read_record(hal, &record);
    if (result == BC2_PIN_SECURITY_NOT_CONFIGURED) return result;
    if (result != BC2_PIN_SECURITY_OK) return result;
    security->configured = true;
    security->failures = record.failures;
    security->blocked_until_ms = now_ms + delay_ms(record.failures);
    secure_clear(&record, sizeof(record));
    return BC2_PIN_SECURITY_OK;
}

bc2_pin_security_result_t bc2_pin_security_create(bc2_pin_security_t *security,
                                                  const bc2_hal_t *hal,
                                                  const char *pin) {
    bc2_pin_record_t record;
    if (security == NULL || hal == NULL || !valid_pin(pin)) return BC2_PIN_SECURITY_INVALID;
    memset(&record, 0, sizeof(record));
    record.magic = BC2_PIN_MAGIC;
    record.version = BC2_PIN_RECORD_VERSION;
    if (bc2_hal_random(hal, record.salt, sizeof(record.salt)) != BC2_HAL_OK ||
        !derive(pin, record.salt, record.hash) ||
        bc2_hal_storage_write(hal, BC2_PIN_RECORD_KEY,
                              (const uint8_t *)&record, sizeof(record)) != BC2_HAL_OK) {
        secure_clear(&record, sizeof(record));
        return BC2_PIN_SECURITY_ERROR;
    }
    secure_clear(&record, sizeof(record));
    security->configured = true;
    security->failures = 0U;
    security->blocked_until_ms = 0U;
    return BC2_PIN_SECURITY_OK;
}

bc2_pin_security_result_t bc2_pin_security_verify(bc2_pin_security_t *security,
                                                  const bc2_hal_t *hal,
                                                  const char *pin,
                                                  uint64_t now_ms) {
    bc2_pin_record_t record;
    uint8_t candidate[BC2_PIN_HASH_SIZE];
    uint8_t difference = 0U;
    size_t index;
    if (security == NULL || hal == NULL || !valid_pin(pin)) return BC2_PIN_SECURITY_INVALID;
    if (now_ms < security->blocked_until_ms) return BC2_PIN_SECURITY_DELAYED;
    if (read_record(hal, &record) != BC2_PIN_SECURITY_OK ||
        !derive(pin, record.salt, candidate)) return BC2_PIN_SECURITY_ERROR;
    for (index = 0U; index < sizeof(candidate); ++index)
        difference |= (uint8_t)(candidate[index] ^ record.hash[index]);
    secure_clear(candidate, sizeof(candidate));
    if (difference == 0U) {
        record.failures = 0U;
        security->failures = 0U;
        security->blocked_until_ms = 0U;
        if (bc2_hal_storage_write(hal, BC2_PIN_RECORD_KEY,
                                  (const uint8_t *)&record, sizeof(record)) != BC2_HAL_OK) {
            secure_clear(&record, sizeof(record));
            return BC2_PIN_SECURITY_ERROR;
        }
        secure_clear(&record, sizeof(record));
        return BC2_PIN_SECURITY_OK;
    }
    if (record.failures < BC2_SECURITY_PIN_MAX_FAILURES) ++record.failures;
    security->failures = record.failures;
    security->blocked_until_ms = now_ms + delay_ms(record.failures);
    if (bc2_hal_storage_write(hal, BC2_PIN_RECORD_KEY,
                              (const uint8_t *)&record, sizeof(record)) != BC2_HAL_OK) {
        secure_clear(&record, sizeof(record));
        return BC2_PIN_SECURITY_ERROR;
    }
    secure_clear(&record, sizeof(record));
    return BC2_PIN_SECURITY_INVALID;
}

bc2_pin_security_result_t bc2_pin_security_reset(bc2_pin_security_t *security, const bc2_hal_t *hal) {
    if (security == NULL || hal == NULL) return BC2_PIN_SECURITY_ERROR;
    const bc2_hal_result_t result = bc2_hal_storage_remove(hal, BC2_PIN_RECORD_KEY);
    if (result != BC2_HAL_OK && result != BC2_HAL_ERROR_NOT_FOUND) return BC2_PIN_SECURITY_ERROR;
    memset(security, 0, sizeof(*security));
    return BC2_PIN_SECURITY_NOT_CONFIGURED;
}

uint64_t bc2_pin_security_remaining_delay(const bc2_pin_security_t *security,
                                          uint64_t now_ms) {
    if (security == NULL || now_ms >= security->blocked_until_ms) return 0U;
    return security->blocked_until_ms - now_ms;
}

bc2_authorization_pin_t bc2_authorization_required_pin(bc2_authorization_action_t action) {
    return action == BC2_AUTH_NEW_WALLET ? BC2_AUTH_ROOT_PIN : BC2_AUTH_DEVICE_PIN;
}
