#include "bc2_receive_request.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static int value_of(char character) {
    static const char alphabet[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
    const char *position = strchr(alphabet, character);
    return position == NULL ? -1 : (int)(position - alphabet);
}

static uint32_t polymod_step(uint32_t previous) {
    const uint8_t top = (uint8_t)(previous >> 25U);
    uint32_t checksum = (previous & 0x1ffffffU) << 5U;
    if ((top & 1U) != 0U) checksum ^= 0x3b6a57b2U;
    if ((top & 2U) != 0U) checksum ^= 0x26508e6dU;
    if ((top & 4U) != 0U) checksum ^= 0x1ea119faU;
    if ((top & 8U) != 0U) checksum ^= 0x3d4233ddU;
    if ((top & 16U) != 0U) checksum ^= 0x2a1462b3U;
    return checksum;
}

bool bc2_receive_address_is_valid(const char *address) {
    const char expected_hrp[] = "bc";
    uint8_t values[84];
    size_t value_count = 0U;
    size_t separator = 0U;
    size_t output_count = 0U;
    unsigned accumulator = 0U;
    unsigned bits = 0U;
    uint32_t checksum = 1U;
    size_t length;

    if (address == NULL) return false;
    length = strlen(address);
    if (length < 14U || length > 90U) return false;
    for (size_t index = 0U; index < length; ++index) {
        if (address[index] < 33 || address[index] > 126 ||
            (address[index] >= 'A' && address[index] <= 'Z')) return false;
        if (address[index] == '1') separator = index;
    }
    if (separator != 2U || separator + 7U > length ||
        address[0] != expected_hrp[0] || address[1] != expected_hrp[1]) return false;
    for (size_t index = 0U; index < separator; ++index)
        checksum = polymod_step(checksum) ^ ((uint32_t)(uint8_t)address[index] >> 5U);
    checksum = polymod_step(checksum);
    for (size_t index = 0U; index < separator; ++index)
        checksum = polymod_step(checksum) ^ ((uint32_t)(uint8_t)address[index] & 31U);
    for (size_t index = separator + 1U; index < length; ++index) {
        const int value = value_of(address[index]);
        if (value < 0) return false;
        values[value_count++] = (uint8_t)value;
        checksum = polymod_step(checksum) ^ (uint32_t)value;
    }
    if (checksum != 1U || value_count < 7U || values[0] != 0U) return false;
    for (size_t index = 1U; index + 6U < value_count; ++index) {
        accumulator = (accumulator << 5U) | values[index];
        bits += 5U;
        while (bits >= 8U) {
            bits -= 8U;
            ++output_count;
        }
    }
    if (bits >= 5U || ((accumulator << (8U - bits)) & 0xffU) != 0U) return false;
    return output_count == 20U;
}
