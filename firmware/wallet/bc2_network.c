#include "bc2_network.h"

#include <stddef.h>
#include <string.h>

/*
 * BC2 currently uses Bitcoin-compatible address encodings.
 *
 * Mainnet:
 *   P2PKH:   0x00
 *   P2SH:    0x05
 *   WIF:     0x80
 *   XPUB:    0x0488B21E
 *   XPRV:    0x0488ADE4
 *   Bech32:  bc
 *   P2P:     8338
 *
 * The dedicated BC2 BIP44 coin type still needs to be treated as a
 * project-level wallet policy until it is formally standardized.
 */
static const bc2_network_t BC2_MAINNET = {
    .name = "Bitcoin II Mainnet",
    .ticker = "BC2",
    .bech32_hrp = "bc",

    .p2pkh_prefix = 0x00,
    .p2sh_prefix = 0x05,
    .wif_prefix = 0x80,

    .xpub_version = 0x0488B21E,
    .xprv_version = 0x0488ADE4,

    .default_p2p_port = 8338,
    .bip44_coin_type = 4541509U
};

static const bc2_network_t BC2_TESTNET = {
    .name = "Bitcoin II Testnet",
    .ticker = "BC2",
    .bech32_hrp = "tb",

    .p2pkh_prefix = 0x6F,
    .p2sh_prefix = 0xC4,
    .wif_prefix = 0xEF,

    .xpub_version = 0x043587CF,
    .xprv_version = 0x04358394,

    .default_p2p_port = 18338,
    .bip44_coin_type = 1U
};

const bc2_network_t *bc2_network_get(const bc2_network_type_t type)
{
    switch (type) {
        case BC2_NETWORK_MAINNET:
            return &BC2_MAINNET;

        case BC2_NETWORK_TESTNET:
            return &BC2_TESTNET;

        default:
            return NULL;
    }
}

bool bc2_network_is_valid(const bc2_network_t *network)
{
    if (network == NULL) {
        return false;
    }

    if (network->name == NULL ||
        network->ticker == NULL ||
        network->bech32_hrp == NULL) {
        return false;
    }

    if (strlen(network->ticker) == 0U ||
        strlen(network->bech32_hrp) == 0U) {
        return false;
    }

    if (network->default_p2p_port == 0U) {
        return false;
    }

    return true;
}