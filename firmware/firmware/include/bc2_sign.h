#ifndef BC2_SIGN_H
#define BC2_SIGN_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
bool bc2_ecdsa_sign_der(const uint8_t[32],const uint8_t[32],uint8_t*,size_t,size_t*);
bool bc2_ecdsa_verify_der(const uint8_t[33],const uint8_t[32],const uint8_t*,size_t);
bool bc2_p2wpkh_sighash_all_multi(
    const uint8_t hash_prevouts[32],
    const uint8_t hash_sequence[32],
    const uint8_t prev_txid_le[32], uint32_t prev_output_index,
    uint64_t input_amount, const uint8_t pubkey_hash[20],
    uint32_t sequence,
    const uint8_t *recipient_script, size_t recipient_script_length,
    uint64_t recipient_amount,
    const uint8_t *change_script, size_t change_script_length,
    uint64_t change_amount, uint32_t lock_time,
    uint8_t output_hash[32]);

bool bc2_p2wpkh_sighash_all_single(
    const uint8_t prev_txid_le[32], uint32_t prev_output_index,
    uint64_t input_amount, const uint8_t pubkey_hash[20],
    uint32_t sequence,
    const uint8_t *recipient_script, size_t recipient_script_length,
    uint64_t recipient_amount,
    const uint8_t *change_script, size_t change_script_length,
    uint64_t change_amount, uint32_t lock_time,
    uint8_t output_hash[32]);
#endif
