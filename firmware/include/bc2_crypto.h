#ifndef BC2_CRYPTO_H
#define BC2_CRYPTO_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define BC2_SHA256_SIZE 32U
#define BC2_SHA512_SIZE 64U
#define BC2_RIPEMD160_SIZE 20U
bool bc2_sha256(const uint8_t*,size_t,uint8_t[BC2_SHA256_SIZE]);
bool bc2_sha256d(const uint8_t*,size_t,uint8_t[BC2_SHA256_SIZE]);
bool bc2_ripemd160(const uint8_t*,size_t,uint8_t[BC2_RIPEMD160_SIZE]);
bool bc2_hash160(const uint8_t*,size_t,uint8_t[BC2_RIPEMD160_SIZE]);
bool bc2_hmac_sha512(const uint8_t*,size_t,const uint8_t*,size_t,uint8_t[BC2_SHA512_SIZE]);
bool bc2_pbkdf2_hmac_sha512(const uint8_t*,size_t,const uint8_t*,size_t,uint32_t,uint8_t*,size_t);
bool bc2_random(uint8_t*,size_t);
#endif
