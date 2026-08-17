#ifndef BC2_WALLET_H
#define BC2_WALLET_H

#include "bc2_network.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BC2_WALLET_MAX_PATH 96U
#define BC2_WALLET_MAX_ADDRESS 96U

typedef enum {
    BC2_WALLET_OK = 0,
    BC2_WALLET_INVALID_ARGUMENT,
    BC2_WALLET_INVALID_MNEMONIC,
    BC2_WALLET_DERIVATION_FAILED,
    BC2_WALLET_ENCODING_FAILED
} bc2_wallet_status;

typedef struct {
    char path[BC2_WALLET_MAX_PATH];
    char address[BC2_WALLET_MAX_ADDRESS];
    uint8_t public_key[33];
} bc2_receive_address;

bc2_wallet_status bc2_wallet_receive_address_from_mnemonic(
    const char *mnemonic,
    const char *passphrase,
    const bc2_network *network,
    uint32_t account,
    uint32_t change,
    uint32_t index,
    bc2_receive_address *result);

const char *bc2_wallet_status_message(bc2_wallet_status status);

#endif
