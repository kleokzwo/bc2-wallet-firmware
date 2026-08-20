#ifndef BC2_HW_WALLET_H
#define BC2_HW_WALLET_H

#include "bc2_hal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_HW_WALLET_WORD_COUNT 12U
#define BC2_HW_WALLET_MAX_WORD_COUNT 24U
#define BC2_HW_WALLET_WORD_SIZE 9U
#define BC2_HW_WALLET_ID_SIZE 16U

typedef enum {
    BC2_HW_WALLET_NONE = 0,
    BC2_HW_WALLET_BACKUP_PENDING = 1,
    BC2_HW_WALLET_READY = 2,
    BC2_HW_WALLET_ERROR = 255
} bc2_hw_wallet_status_t;

bc2_hw_wallet_status_t bc2_hw_wallet_status(const bc2_hal_t *hal);
bool bc2_hw_wallet_create(const bc2_hal_t *hal,
                          char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]);
bool bc2_hw_wallet_load_words(const bc2_hal_t *hal,
                              char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]);
bool bc2_hw_wallet_confirm_backup(const bc2_hal_t *hal);
bool bc2_hw_wallet_validate_indexes(const uint16_t *indexes, size_t word_count);
bool bc2_hw_wallet_restore_indexes(const bc2_hal_t *hal, const uint16_t *indexes, size_t word_count);
bool bc2_hw_wallet_factory_reset(const bc2_hal_t *hal);
bool bc2_hw_wallet_id(const bc2_hal_t *hal,
                      uint8_t wallet_id[BC2_HW_WALLET_ID_SIZE]);

/* Derive a public receive address from the encrypted hardware seed.
 * No seed/private-key material leaves this function. */
bool bc2_hw_wallet_receive_address(const bc2_hal_t *hal, uint32_t index,
                                   char *address, size_t address_capacity);
bool bc2_hw_wallet_receive_index(const bc2_hal_t *hal, uint32_t *index);
bool bc2_hw_wallet_commit_receive_index(const bc2_hal_t *hal, uint32_t index);
bool bc2_hw_wallet_sign_single_p2wpkh(
 const bc2_hal_t *hal,const char *input_address,const uint8_t prev_txid_le[32],uint32_t prev_output_index,
 uint64_t input_amount,uint32_t sequence,const char *recipient_address,uint64_t recipient_amount,
 uint64_t change_amount,uint32_t lock_time,uint8_t public_key[33],uint8_t *signature,
 size_t signature_capacity,size_t *signature_length);

void bc2_hw_wallet_clear_words(
    char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
