#ifndef BC2_HEX_H
#define BC2_HEX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t bc2_hex_encoded_size(size_t input_length);

bool bc2_hex_encode(
    const uint8_t *input,
    size_t input_length,
    char *output,
    size_t output_size);

bool bc2_hex_decode(
    const char *input,
    uint8_t *output,
    size_t output_size,
    size_t *decoded_length);

#endif