#include "bc2_network.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_mainnet_parameters(void)
{
    const bc2_network_t *network =
        bc2_network_get(BC2_NETWORK_MAINNET);

    assert(network != NULL);
    assert(bc2_network_is_valid(network));

    assert(strcmp(network->name, "Bitcoin II Mainnet") == 0);
    assert(strcmp(network->ticker, "BC2") == 0);
    assert(strcmp(network->bech32_hrp, "bc") == 0);

    assert(network->p2pkh_prefix == 0x00);
    assert(network->p2sh_prefix == 0x05);
    assert(network->wif_prefix == 0x80);

    assert(network->xpub_version == 0x0488B21E);
    assert(network->xprv_version == 0x0488ADE4);

    assert(network->default_p2p_port == 8338);
    assert(network->bip44_coin_type == 4541509U);
}

static void test_invalid_network(void)
{
    const bc2_network_t *network =
        bc2_network_get((bc2_network_type_t)99);

    assert(network == NULL);
    assert(!bc2_network_is_valid(NULL));
}

int main(void)
{
    test_mainnet_parameters();
    test_invalid_network();

    puts("All BC2 network tests passed.");
    return 0;
}