#include "bc2_wallet.h"

#include <assert.h>
#include <string.h>

int main(void) {
    static const char *TEST_VECTOR =
        "abandon abandon abandon abandon abandon abandon abandon abandon "
        "abandon abandon abandon about";
    bc2_receive_address receive;
    bc2_wallet_status status = bc2_wallet_receive_address_from_mnemonic(
        TEST_VECTOR, "", bc2_network_mainnet(), 0U, 0U, 0U, &receive);

    assert(status == BC2_WALLET_OK);
    assert(strcmp(receive.path, "m/84'/4541509'/0'/0/0") == 0);
    assert(strncmp(receive.address, "bc1", 3U) == 0);
    assert(strlen(receive.address) > 20U);

    status = bc2_wallet_receive_address_from_mnemonic(
        "not a valid mnemonic", "", bc2_network_mainnet(), 0U, 0U, 0U, &receive);
    assert(status == BC2_WALLET_INVALID_MNEMONIC);

    status = bc2_wallet_receive_address_from_mnemonic(
        TEST_VECTOR, "", bc2_network_mainnet(), 0U, 2U, 0U, &receive);
    assert(status == BC2_WALLET_INVALID_ARGUMENT);
    return 0;
}
