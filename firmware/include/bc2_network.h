#ifndef BC2_NETWORK_H
#define BC2_NETWORK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BC2_NETWORK_MAINNET = 0,
    BC2_NETWORK_TESTNET
} bc2_network_type_t;

typedef struct {
    const char *name;
    const char *ticker;
    const char *bech32_hrp;

    uint8_t p2pkh_prefix;
    uint8_t p2sh_prefix;
    uint8_t wif_prefix;

    uint32_t xpub_version;
    uint32_t xprv_version;

    uint16_t default_p2p_port;
    uint32_t bip44_coin_type;
} bc2_network_t;

const bc2_network_t *bc2_network_get(bc2_network_type_t type);

bool bc2_network_is_valid(const bc2_network_t *network);

#endif