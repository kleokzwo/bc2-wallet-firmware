#include "bc2_network.h"

static const bc2_network MAINNET = {
    "BC2 Mainnet", "bc", 0x00U, 0x0488ADE4U, 0x0488B21EU, 4541509U
};

static const bc2_network TESTNET = {
    "BC2 Testnet", "tb", 0x6FU, 0x04358394U, 0x043587CFU, 1U
};

const bc2_network *bc2_network_mainnet(void) { return &MAINNET; }
const bc2_network *bc2_network_testnet(void) { return &TESTNET; }
