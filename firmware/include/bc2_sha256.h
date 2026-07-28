#ifndef BC2_SHA256_H
#define BC2_SHA256_H

#include <stddef.h>
#include <stdint.h>

#define BC2_SHA256_DIGEST_SIZE 32U
#define BC2_SHA256_BLOCK_SIZE 64U

typedef struct
{
  uint32_t state[8];
  uint64_t total_length;
  uint8_t buffer[BC2_SHA256_BLOCK_SIZE];
  size_t buffer_length;
} bc2_sha256_context_t;

void bc2_sha256_init(bc2_sha256_context_t *context);

void bc2_sha256_update(
    bc2_sha256_context_t *context,
    const uint8_t *data,
    size_t data_length);

void bc2_sha256_final(
    bc2_sha256_context_t *context,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE]);

void bc2_sha256(
    const uint8_t *data,
    size_t data_length,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE]);

#endif