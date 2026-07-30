#include "bc2_psbt.h"
#include <string.h>

#define BC2_PSBT_MAX_SIZE (1024U * 1024U)
#define BC2_PSBT_MAX_GLOBAL_PAIRS 128U

static int read_compact_size(const uint8_t *data, size_t size, size_t *offset, uint64_t *value) {
    uint8_t first;
    if (*offset >= size) return 0;
    first = data[(*offset)++];
    if (first < 0xfdU) { *value = first; return 1; }
    if (first == 0xfdU) {
        if (size - *offset < 2U) return 0;
        *value = (uint64_t)data[*offset] | ((uint64_t)data[*offset + 1U] << 8U);
        *offset += 2U; return 1;
    }
    if (first == 0xfeU) {
        if (size - *offset < 4U) return 0;
        *value = (uint64_t)data[*offset] | ((uint64_t)data[*offset + 1U] << 8U) |
                 ((uint64_t)data[*offset + 2U] << 16U) | ((uint64_t)data[*offset + 3U] << 24U);
        *offset += 4U; return 1;
    }
    if (size - *offset < 8U) return 0;
    *value = 0U;
    for (unsigned int i = 0U; i < 8U; ++i) *value |= ((uint64_t)data[*offset + i]) << (8U * i);
    *offset += 8U; return 1;
}

bc2_psbt_status bc2_psbt_inspect(const uint8_t *data, size_t size, bc2_psbt_summary *summary) {
    static const uint8_t magic[5] = {'p','s','b','t',0xff};
    size_t offset = 5U;
    unsigned int pairs = 0U;
    if (data == 0 || summary == 0) return BC2_PSBT_INVALID_ARGUMENT;
    memset(summary, 0, sizeof(*summary));
    summary->total_size = size;
    if (size > BC2_PSBT_MAX_SIZE) return BC2_PSBT_LIMIT_EXCEEDED;
    if (size < sizeof(magic)) return BC2_PSBT_TRUNCATED;
    if (memcmp(data, magic, sizeof(magic)) != 0) return BC2_PSBT_INVALID_MAGIC;

    while (offset < size) {
        uint64_t key_len = 0U, value_len = 0U;
        size_t key_start;
        if (!read_compact_size(data, size, &offset, &key_len)) return BC2_PSBT_TRUNCATED;
        if (key_len == 0U) {
            summary->global_key_value_pairs = pairs;
            summary->structurally_valid = 1;
            return summary->contains_unsigned_transaction ? BC2_PSBT_OK : BC2_PSBT_UNSUPPORTED;
        }
        if (key_len > (uint64_t)(size - offset)) return BC2_PSBT_TRUNCATED;
        key_start = offset;
        offset += (size_t)key_len;
        if (!read_compact_size(data, size, &offset, &value_len)) return BC2_PSBT_TRUNCATED;
        if (value_len > (uint64_t)(size - offset)) return BC2_PSBT_TRUNCATED;
        if (data[key_start] == 0x00U && key_len == 1U) summary->contains_unsigned_transaction = 1;
        offset += (size_t)value_len;
        ++pairs;
        if (pairs > BC2_PSBT_MAX_GLOBAL_PAIRS) return BC2_PSBT_LIMIT_EXCEEDED;
    }
    return BC2_PSBT_TRUNCATED;
}

const char *bc2_psbt_status_message(bc2_psbt_status status) {
    switch (status) {
        case BC2_PSBT_OK: return "PSBT structure accepted";
        case BC2_PSBT_INVALID_ARGUMENT: return "Invalid argument";
        case BC2_PSBT_INVALID_MAGIC: return "Invalid PSBT magic";
        case BC2_PSBT_TRUNCATED: return "Truncated PSBT";
        case BC2_PSBT_UNSUPPORTED: return "Unsupported PSBT structure";
        case BC2_PSBT_LIMIT_EXCEEDED: return "PSBT safety limit exceeded";
        default: return "Unknown PSBT status";
    }
}
