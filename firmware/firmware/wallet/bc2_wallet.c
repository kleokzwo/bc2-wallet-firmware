#include "bc2_wallet.h"

#include "bc2_address.h"
#include "bc2_bip32.h"
#include "bc2_bip39.h"
#include "bc2_secure.h"

#include <stdio.h>
#include <string.h>

bc2_wallet_status bc2_wallet_receive_address_from_mnemonic(
    const char *mnemonic,
    const char *passphrase,
    const bc2_network *network,
    uint32_t account,
    uint32_t change,
    uint32_t index,
    bc2_receive_address *result) {
    uint8_t seed[64] = {0};
    bc2_xprv master = {0};
    bc2_xprv node = {0};
    int written;
    bc2_wallet_status status = BC2_WALLET_DERIVATION_FAILED;

    if (!mnemonic || !passphrase || !network || !result || change > 1U ||
        account > 0x7FFFFFFFU || index > 0x7FFFFFFFU) {
        return BC2_WALLET_INVALID_ARGUMENT;
    }

    memset(result, 0, sizeof(*result));
    if (!bc2_bip39_validate(mnemonic)) {
        return BC2_WALLET_INVALID_MNEMONIC;
    }

    written = snprintf(result->path, sizeof(result->path), "m/84'/%u'/%u'/%u/%u",
                       network->coin_type, account, change, index);
    if (written < 0 || (size_t)written >= sizeof(result->path)) {
        return BC2_WALLET_INVALID_ARGUMENT;
    }

    if (!bc2_bip39_seed(mnemonic, passphrase, seed) ||
        !bc2_bip32_master(seed, sizeof(seed), &master) ||
        !bc2_bip32_derive_path(&master, result->path, &node) ||
        !bc2_secp256k1_public(node.key, result->public_key)) {
        goto cleanup;
    }

    if (!bc2_address_p2wpkh(result->public_key, network->bech32_hrp,
                            result->address, sizeof(result->address))) {
        status = BC2_WALLET_ENCODING_FAILED;
        goto cleanup;
    }

    status = BC2_WALLET_OK;

cleanup:
    bc2_secure_zero(seed, sizeof(seed));
    bc2_secure_zero(&master, sizeof(master));
    bc2_secure_zero(&node, sizeof(node));
    if (status != BC2_WALLET_OK) {
        bc2_secure_zero(result, sizeof(*result));
    }
    return status;
}

const char *bc2_wallet_status_message(bc2_wallet_status status) {
    switch (status) {
        case BC2_WALLET_OK: return "OK";
        case BC2_WALLET_INVALID_ARGUMENT: return "Ungültige Eingabe";
        case BC2_WALLET_INVALID_MNEMONIC: return "Ungültige Test-Mnemonic";
        case BC2_WALLET_DERIVATION_FAILED: return "Schlüsselableitung fehlgeschlagen";
        case BC2_WALLET_ENCODING_FAILED: return "Adresskodierung fehlgeschlagen";
        default: return "Unbekannter Fehler";
    }
}
