#include "bc2_psbt.h"
#include "bc2_network.h"
#include "bc2_receive_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return EXIT_FAILURE; } while (0)

int main(void) {
    static const char path[] = BC2_SOURCE_DIR "/test-data/bc2-safe-test-transaction.psbt";
    uint8_t data[512];
    bc2_psbt_summary summary;
    bc2_owned_script wallet_script = {0};
    FILE *file = fopen(path, "rb");
    size_t length;

    CHECK(file != NULL);
    length = fread(data, 1U, sizeof(data), file);
    CHECK(fclose(file) == 0);
    CHECK(length == 127U);
    CHECK(bc2_psbt_review(data, length, NULL, 0U,
                          bc2_network_mainnet()->bech32_hrp, &summary) == BC2_PSBT_OK);
    CHECK(summary.input_count == 1U);
    CHECK(summary.output_count == 1U);
    CHECK(summary.total_input_amount == 100000U);
    CHECK(summary.total_output_amount == 99000U);
    CHECK(summary.external_output_amount == 99000U);
    CHECK(summary.change_amount == 0U);
    CHECK(summary.fee_amount == 1000U);
    CHECK(summary.outputs[0].address[0] != '\0');
    CHECK(bc2_receive_address_is_valid(summary.outputs[0].address));

    wallet_script.length = 22U;
    wallet_script.is_change = 1;
    wallet_script.bytes[0] = 0x00U;
    wallet_script.bytes[1] = 0x14U;
    memset(wallet_script.bytes + 2U, 0x22, 20U);
    CHECK(bc2_psbt_review(data, length, &wallet_script, 1U,
                          bc2_network_mainnet()->bech32_hrp, &summary) == BC2_PSBT_OK);
    CHECK(summary.change_verified == 1);
    CHECK(summary.change_amount == 0U);
    CHECK(summary.external_output_amount == 99000U);
    return EXIT_SUCCESS;
}
