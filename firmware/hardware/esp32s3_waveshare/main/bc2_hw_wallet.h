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

/* Derive a public receive address from the encrypted hardware seed.
 * No seed/private-key material leaves this function. */
bool bc2_hw_wallet_receive_address(const bc2_hal_t *hal, uint32_t index,
                                   char *address, size_t address_capacity);
bool bc2_hw_wallet_receive_index(const bc2_hal_t *hal, uint32_t *index);
bool bc2_hw_wallet_commit_receive_index(const bc2_hal_t *hal, uint32_t index);

void bc2_hw_wallet_clear_words(
    char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif
