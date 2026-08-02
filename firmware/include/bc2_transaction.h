#ifndef BC2_TRANSACTION_H
#define BC2_TRANSACTION_H

#include <stddef.h>
#include <stdint.h>
#include "bc2_network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_TX_MAX_INPUTS 64U
#define BC2_TX_MAX_SCRIPT_SIZE 128U

typedef enum {
    BC2_TX_OK = 0,
    BC2_TX_INVALID_ARGUMENT,
    BC2_TX_INVALID_ADDRESS,
    BC2_TX_INSUFFICIENT_FUNDS,
    BC2_TX_LIMIT_EXCEEDED,
    BC2_TX_BUFFER_TOO_SMALL,
    BC2_TX_VALUE_OVERFLOW
} bc2_tx_status;

typedef struct {
    uint8_t txid[32];
    uint32_t output_index;
    uint64_t amount;
    uint8_t script[BC2_TX_MAX_SCRIPT_SIZE];
    size_t script_length;
} bc2_tx_utxo;

typedef struct {
    unsigned int selected_input_count;
    unsigned int selected_indices[BC2_TX_MAX_INPUTS];
    uint64_t selected_amount;
    uint64_t recipient_amount;
    uint64_t fee_amount;
    uint64_t change_amount;
    uint64_t fee_rate_sat_vbyte;
    size_t estimated_vbytes;
    int change_output_created;
} bc2_tx_plan;

bc2_tx_status bc2_address_to_script(const char *address, const bc2_network *network,
                                    uint8_t *script, size_t script_capacity,
                                    size_t *script_length);

bc2_tx_status bc2_transaction_plan(const bc2_tx_utxo *utxos, size_t utxo_count,
                                   uint64_t recipient_amount,
                                   uint64_t fee_rate_sat_vbyte,
                                   uint64_t dust_limit,
                                   bc2_tx_plan *plan);

bc2_tx_status bc2_psbt_create(const bc2_tx_utxo *utxos, size_t utxo_count,
                              const bc2_tx_plan *plan,
                              const uint8_t *recipient_script, size_t recipient_script_length,
                              const uint8_t *change_script, size_t change_script_length,
                              uint8_t *output, size_t output_capacity, size_t *output_length);

const char *bc2_tx_status_message(bc2_tx_status status);

#ifdef __cplusplus
}
#endif
#endif
