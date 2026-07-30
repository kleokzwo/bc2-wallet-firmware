#ifndef BC2_NETWORK_H
#define BC2_NETWORK_H

#include <stdint.h>

typedef struct {
    const char *name;
    const char *bech32_hrp;
    uint8_t p2pkh_prefix;
    uint32_t xprv_version;
    uint32_t xpub_version;
    uint32_t coin_type;
} bc2_network;

const bc2_network *bc2_network_mainnet(void);
const bc2_network *bc2_network_testnet(void);

#endif
