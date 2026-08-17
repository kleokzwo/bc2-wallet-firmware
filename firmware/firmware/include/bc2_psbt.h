#ifndef BC2_PSBT_H
#define BC2_PSBT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_PSBT_MAX_INPUTS 64U
#define BC2_PSBT_MAX_OUTPUTS 64U
#define BC2_PSBT_MAX_SCRIPT_SIZE 128U
#define BC2_PSBT_MAX_ADDRESS_SIZE 96U
#define BC2_PSBT_MAX_OWNED_SCRIPTS 128U

typedef enum {
    BC2_PSBT_OK = 0,
    BC2_PSBT_INVALID_ARGUMENT,
    BC2_PSBT_INVALID_MAGIC,
    BC2_PSBT_TRUNCATED,
    BC2_PSBT_MALFORMED,
    BC2_PSBT_UNSUPPORTED,
    BC2_PSBT_LIMIT_EXCEEDED,
    BC2_PSBT_MISSING_UTXO,
    BC2_PSBT_VALUE_OVERFLOW
} bc2_psbt_status;

typedef struct {
    uint8_t bytes[BC2_PSBT_MAX_SCRIPT_SIZE];
    size_t length;
    int is_change;
} bc2_owned_script;

typedef struct {
    uint8_t previous_txid[32];
    uint32_t previous_output_index;
    uint32_t sequence;
    uint64_t amount;
    int amount_known;
} bc2_psbt_input;

typedef struct {
    uint64_t amount;
    uint8_t script[BC2_PSBT_MAX_SCRIPT_SIZE];
    size_t script_length;
    int owned;
    int change;
    char address[BC2_PSBT_MAX_ADDRESS_SIZE];
} bc2_psbt_output;

typedef struct {
    size_t total_size;
    unsigned int global_key_value_pairs;
    unsigned int input_count;
    unsigned int output_count;
    uint32_t transaction_version;
    uint32_t lock_time;
    uint64_t total_input_amount;
    uint64_t total_output_amount;
    uint64_t external_output_amount;
    uint64_t change_amount;
    uint64_t fee_amount;
    int contains_unsigned_transaction;
    int structurally_valid;
    int all_input_amounts_known;
    int fee_known;
    int change_verified;
    bc2_psbt_input inputs[BC2_PSBT_MAX_INPUTS];
    bc2_psbt_output outputs[BC2_PSBT_MAX_OUTPUTS];
} bc2_psbt_summary;

bc2_psbt_status bc2_psbt_inspect(const uint8_t *data, size_t size, bc2_psbt_summary *summary);
bc2_psbt_status bc2_psbt_review(const uint8_t *data, size_t size,
                                const bc2_owned_script *owned_scripts, size_t owned_script_count,
                                const char *bech32_hrp, bc2_psbt_summary *summary);
const char *bc2_psbt_status_message(bc2_psbt_status status);

#ifdef __cplusplus
}
#endif
#endif
