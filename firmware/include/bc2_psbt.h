#ifndef BC2_PSBT_H
#define BC2_PSBT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BC2_PSBT_OK = 0,
    BC2_PSBT_INVALID_ARGUMENT,
    BC2_PSBT_INVALID_MAGIC,
    BC2_PSBT_TRUNCATED,
    BC2_PSBT_UNSUPPORTED,
    BC2_PSBT_LIMIT_EXCEEDED
} bc2_psbt_status;

typedef struct {
    size_t total_size;
    unsigned int global_key_value_pairs;
    int contains_unsigned_transaction;
    int structurally_valid;
} bc2_psbt_summary;

bc2_psbt_status bc2_psbt_inspect(const uint8_t *data, size_t size, bc2_psbt_summary *summary);
const char *bc2_psbt_status_message(bc2_psbt_status status);

#ifdef __cplusplus
}
#endif

#endif
