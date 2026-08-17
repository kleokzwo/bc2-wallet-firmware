#include "bc2_crypto.h"

#include <limits.h>
#include <string.h>

#ifdef ESP_PLATFORM

#include "esp_random.h"
#include "mbedtls/md.h"

static bool digest(mbedtls_md_type_t type, const uint8_t *data, size_t size,
                   uint8_t *output, size_t expected_size) {
    const mbedtls_md_info_t *info;
    if ((size > 0U && data == NULL) || output == NULL) return false;
    info = mbedtls_md_info_from_type(type);
    if (info == NULL || mbedtls_md_get_size(info) != expected_size) return false;
    return mbedtls_md(info, data, size, output) == 0;
}

bool bc2_sha256(const uint8_t *data, size_t size, uint8_t output[32]) {
    return digest(MBEDTLS_MD_SHA256, data, size, output, 32U);
}

bool bc2_sha256d(const uint8_t *data, size_t size, uint8_t output[32]) {
    uint8_t temporary[32];
    bool ok = bc2_sha256(data, size, temporary) &&
              bc2_sha256(temporary, sizeof(temporary), output);
    memset(temporary, 0, sizeof(temporary));
    return ok;
}

bool bc2_ripemd160(const uint8_t *data, size_t size, uint8_t output[20]) {
    return digest(MBEDTLS_MD_RIPEMD160, data, size, output, 20U);
}

bool bc2_hash160(const uint8_t *data, size_t size, uint8_t output[20]) {
    uint8_t temporary[32];
    bool ok = bc2_sha256(data, size, temporary) &&
              bc2_ripemd160(temporary, sizeof(temporary), output);
    memset(temporary, 0, sizeof(temporary));
    return ok;
}

bool bc2_hmac_sha512(const uint8_t *key, size_t key_size,
                     const uint8_t *data, size_t data_size,
                     uint8_t output[64]) {
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);
    if ((key_size > 0U && key == NULL) || (data_size > 0U && data == NULL) ||
        output == NULL || info == NULL)
        return false;
    return mbedtls_md_hmac(info, key, key_size, data, data_size, output) == 0;
}

/* BIP39 needs PBKDF2-HMAC-SHA512 with 2048 rounds.  SHA-512's output length is
 * already exactly the requested 64-byte seed, so only PBKDF2 block #1 exists.
 * Keeping this tiny implementation here avoids filesystem or desktop-library
 * dependencies on the hardware. */
bool bc2_pbkdf2_hmac_sha512(const uint8_t *password, size_t password_size,
                            const uint8_t *salt, size_t salt_size,
                            uint32_t iterations, uint8_t *output,
                            size_t output_size) {
    uint8_t block_input[260];
    uint8_t u[64];
    uint8_t next[64];
    uint32_t round;
    size_t index;

    if ((password_size > 0U && password == NULL) ||
        (salt_size > 0U && salt == NULL) ||
        output == NULL || output_size != 64U || iterations == 0U ||
        salt_size + 4U > sizeof(block_input))
        return false;

    if (salt_size > 0U) memcpy(block_input, salt, salt_size);
    block_input[salt_size + 0U] = 0U;
    block_input[salt_size + 1U] = 0U;
    block_input[salt_size + 2U] = 0U;
    block_input[salt_size + 3U] = 1U;

    if (!bc2_hmac_sha512(password, password_size,
                         block_input, salt_size + 4U, u)) {
        memset(block_input, 0, sizeof(block_input));
        return false;
    }
    memcpy(output, u, sizeof(u));

    for (round = 1U; round < iterations; ++round) {
        if (!bc2_hmac_sha512(password, password_size, u, sizeof(u), next)) {
            memset(block_input, 0, sizeof(block_input));
            memset(u, 0, sizeof(u));
            memset(next, 0, sizeof(next));
            memset(output, 0, output_size);
            return false;
        }
        for (index = 0U; index < sizeof(next); ++index)
            output[index] ^= next[index];
        memcpy(u, next, sizeof(u));
    }

    memset(block_input, 0, sizeof(block_input));
    memset(u, 0, sizeof(u));
    memset(next, 0, sizeof(next));
    return true;
}

bool bc2_random(uint8_t *output, size_t size) {
    if (output == NULL) return false;
    esp_fill_random(output, size);
    return true;
}

#else

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

static bool digest(const EVP_MD *md,const uint8_t *d,size_t n,uint8_t *out,unsigned int want){
 if((n>0U&&d==NULL)||out==NULL||n>(size_t)INT_MAX)return false;
 EVP_MD_CTX *c=EVP_MD_CTX_new(); if(!c)return false; unsigned int got=0; bool ok=EVP_DigestInit_ex(c,md,NULL)==1&&EVP_DigestUpdate(c,d,n)==1&&EVP_DigestFinal_ex(c,out,&got)==1&&got==want; EVP_MD_CTX_free(c); return ok;
}
bool bc2_sha256(const uint8_t*d,size_t n,uint8_t o[32]){return digest(EVP_sha256(),d,n,o,32U);}
bool bc2_sha256d(const uint8_t*d,size_t n,uint8_t o[32]){uint8_t t[32]; if(!bc2_sha256(d,n,t))return false; return bc2_sha256(t,32,o);}
bool bc2_ripemd160(const uint8_t*d,size_t n,uint8_t o[20]){return digest(EVP_ripemd160(),d,n,o,20U);}
bool bc2_hash160(const uint8_t*d,size_t n,uint8_t o[20]){uint8_t t[32]; return bc2_sha256(d,n,t)&&bc2_ripemd160(t,32,o);}
bool bc2_hmac_sha512(const uint8_t*k,size_t kn,const uint8_t*d,size_t n,uint8_t o[64]){if((kn&& !k)||(n&&!d)||!o||kn>(size_t)INT_MAX)return false; unsigned int got=0; return HMAC(EVP_sha512(),k,(int)kn,d,n,o,&got)!=NULL&&got==64U;}
bool bc2_pbkdf2_hmac_sha512(const uint8_t*p,size_t pn,const uint8_t*s,size_t sn,uint32_t it,uint8_t*o,size_t on){if((pn&&!p)||(sn&&!s)||!o||pn>(size_t)INT_MAX||sn>(size_t)INT_MAX||on>(size_t)INT_MAX||it==0U||it>(uint32_t)INT_MAX)return false;return PKCS5_PBKDF2_HMAC((const char*)p,(int)pn,s,(int)sn,(int)it,EVP_sha512(),(int)on,o)==1;}
bool bc2_random(uint8_t*o,size_t n){if(!o||n>(size_t)INT_MAX)return false;return RAND_bytes(o,(int)n)==1;}

#endif
