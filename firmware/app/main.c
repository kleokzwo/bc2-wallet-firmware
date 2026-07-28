#include "bc2_derivation.h"
#include "bc2_network.h"

#include <inttypes.h>
#include <stdio.h>

int main(void)
{
  const bc2_network_t *network =
      bc2_network_get(BC2_NETWORK_MAINNET);

  const bc2_wallet_policy_t *policy =
      bc2_wallet_policy_get(
          BC2_POLICY_BITCOIN_COMPATIBLE);

  bc2_derivation_path_t path;
  char path_text[128];

  if (!bc2_network_is_valid(network) ||
      policy == NULL)
  {
    fprintf(
        stderr,
        "BC2 configuration is invalid.\n");
    return 1;
  }

  if (!bc2_derivation_address_path(
          policy,
          0U,
          0U,
          &path) ||
      !bc2_derivation_format(
          &path,
          path_text,
          sizeof(path_text)))
  {
    fprintf(
        stderr,
        "Could not create derivation path.\n");
    return 1;
  }

  printf("BC2 Wallet Firmware\n");
  printf("-------------------\n");
  printf("Network: %s\n", network->name);
  printf("Ticker:  %s\n", network->ticker);
  printf("Policy:  %s\n", policy->name);
  printf("Path:    %s\n", path_text);

  return 0;
}