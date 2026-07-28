#ifndef BC2_HASH_H
#define BC2_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "bc2_sha256.h"

void bc2_sha256d(
    const uint8_t *data,
    size_t data_length,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE]);

void bc2_sha256d_checksum(
    const uint8_t *data,
    size_t data_length,
    uint8_t checksum[4]);

#endif