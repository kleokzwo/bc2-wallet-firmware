#include "bc2_network.h"

#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    const bc2_network_t *network =
        bc2_network_get(BC2_NETWORK_MAINNET);

    if (!bc2_network_is_valid(network)) {
        fprintf(stderr, "BC2 network configuration is invalid.\n");
        return 1;
    }

    printf("BC2 Wallet Firmware\n");
    printf("-------------------\n");
    printf("Network:  %s\n", network->name);
    printf("Ticker:   %s\n", network->ticker);
    printf("Bech32:   %s\n", network->bech32_hrp);
    printf("P2P port: %" PRIu16 "\n", network->default_p2p_port);
    printf("Coin type: %" PRIu32 "\n", network->bip44_coin_type);

    return 0;
}