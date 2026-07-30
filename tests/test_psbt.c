#include "bc2_psbt.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CHECK failed: %s\n", #x); return EXIT_FAILURE; } } while (0)
int main(void) {
    const unsigned char minimal[] = {'p','s','b','t',0xff, 0x01,0x00, 0x01,0x00, 0x00};
    bc2_psbt_summary summary;
    CHECK(bc2_psbt_inspect(minimal, sizeof(minimal), &summary) == BC2_PSBT_OK);
    CHECK(summary.structurally_valid == 1);
    CHECK(summary.contains_unsigned_transaction == 1);
    CHECK(summary.global_key_value_pairs == 1U);
    CHECK(bc2_psbt_inspect((const unsigned char *)"bad", 3U, &summary) == BC2_PSBT_TRUNCATED);
    return EXIT_SUCCESS;
}
