#include "bc2_derivation.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_bitcoin_compatible_path(void)
{
  const bc2_wallet_policy_t *policy =
      bc2_wallet_policy_get(
          BC2_POLICY_BITCOIN_COMPATIBLE);

  bc2_derivation_path_t path;
  char formatted[128];

  assert(policy != NULL);

  assert(
      bc2_derivation_address_path(
          policy,
          0U,
          0U,
          &path));

  assert(path.depth == 5U);

  assert(
      bc2_derivation_format(
          &path,
          formatted,
          sizeof(formatted)));

  assert(
      strcmp(
          formatted,
          "m/84'/0'/0'/0/0") == 0);

  assert(
      bc2_derivation_is_allowed(policy, &path));
}

static void test_bc2_native_path(void)
{
  const bc2_wallet_policy_t *policy =
      bc2_wallet_policy_get(BC2_POLICY_NATIVE);

  bc2_derivation_path_t path;
  char formatted[128];

  assert(policy != NULL);

  assert(
      bc2_derivation_address_path(
          policy,
          0U,
          15U,
          &path));

  assert(
      bc2_derivation_format(
          &path,
          formatted,
          sizeof(formatted)));

  assert(
      strcmp(
          formatted,
          "m/84'/4541509'/0'/0/15") == 0);

  assert(
      bc2_derivation_is_allowed(policy, &path));
}

static void test_legacy_path(void)
{
  const bc2_wallet_policy_t *policy =
      bc2_wallet_policy_get(BC2_POLICY_LEGACY);

  bc2_derivation_path_t path;
  char formatted[128];

  assert(
      bc2_derivation_address_path(
          policy,
          1U,
          7U,
          &path));

  assert(
      bc2_derivation_format(
          &path,
          formatted,
          sizeof(formatted)));

  assert(
      strcmp(
          formatted,
          "m/44'/0'/0'/1/7") == 0);

  assert(
      bc2_derivation_is_allowed(policy, &path));
}

static void test_parse_path(void)
{
  bc2_derivation_path_t path;
  char formatted[128];

  assert(
      bc2_derivation_parse(
          "m/84'/4541509'/0'/1/42",
          &path));

  assert(path.depth == 5U);
  assert(
      bc2_derivation_is_hardened(
          path.components[0]));

  assert(
      bc2_derivation_unharden(
          path.components[1]) == 4541509U);

  assert(
      bc2_derivation_format(
          &path,
          formatted,
          sizeof(formatted)));

  assert(
      strcmp(
          formatted,
          "m/84'/4541509'/0'/1/42") == 0);
}

static void test_invalid_paths(void)
{
  bc2_derivation_path_t path;

  assert(
      !bc2_derivation_parse(NULL, &path));

  assert(
      !bc2_derivation_parse(
          "84'/0'/0'/0/0",
          &path));

  assert(
      !bc2_derivation_parse(
          "m/84'/abc'/0'/0/0",
          &path));

  assert(
      !bc2_derivation_parse(
          "m/84'/0'/0'/2/0",
          &path));

  const bc2_wallet_policy_t *policy =
      bc2_wallet_policy_get(
          BC2_POLICY_BITCOIN_COMPATIBLE);

  assert(
      bc2_derivation_parse(
          "m/84'/0'/0'/2/0",
          &path));

  assert(
      !bc2_derivation_is_allowed(
          policy,
          &path));
}

static void test_cross_policy_rejection(void)
{
  bc2_derivation_path_t path;

  const bc2_wallet_policy_t *native_policy =
      bc2_wallet_policy_get(BC2_POLICY_NATIVE);

  const bc2_wallet_policy_t *compatible_policy =
      bc2_wallet_policy_get(
          BC2_POLICY_BITCOIN_COMPATIBLE);

  assert(
      bc2_derivation_parse(
          "m/84'/4541509'/0'/0/0",
          &path));

  assert(
      bc2_derivation_is_allowed(
          native_policy,
          &path));

  assert(
      !bc2_derivation_is_allowed(
          compatible_policy,
          &path));
}

int main(void)
{
  test_bitcoin_compatible_path();
  test_bc2_native_path();
  test_legacy_path();
  test_parse_path();
  test_invalid_paths();
  test_cross_policy_rejection();

  puts("All BC2 derivation tests passed.");
  return 0;
}