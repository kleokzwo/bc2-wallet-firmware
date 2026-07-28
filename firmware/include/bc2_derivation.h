#ifndef BC2_DERIVATION_H
#define BC2_DERIVATION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BC2_DERIVATION_MAX_DEPTH 10U
#define BC2_DERIVATION_HARDENED 0x80000000UL

typedef enum
{
  BC2_SCRIPT_LEGACY_P2PKH = 0,
  BC2_SCRIPT_NATIVE_SEGWIT
} bc2_script_type_t;

typedef enum
{
  BC2_POLICY_BITCOIN_COMPATIBLE = 0,
  BC2_POLICY_NATIVE,
  BC2_POLICY_LEGACY
} bc2_wallet_policy_type_t;

typedef struct
{
  uint32_t components[BC2_DERIVATION_MAX_DEPTH];
  size_t depth;
} bc2_derivation_path_t;

typedef struct
{
  const char *name;
  bc2_wallet_policy_type_t type;
  bc2_script_type_t script_type;
  uint32_t purpose;
  uint32_t coin_type;
  uint32_t account;
} bc2_wallet_policy_t;

const bc2_wallet_policy_t *
bc2_wallet_policy_get(bc2_wallet_policy_type_t type);

bool bc2_derivation_account_path(
    const bc2_wallet_policy_t *policy,
    bc2_derivation_path_t *output);

bool bc2_derivation_address_path(
    const bc2_wallet_policy_t *policy,
    uint32_t change,
    uint32_t address_index,
    bc2_derivation_path_t *output);

bool bc2_derivation_parse(
    const char *text,
    bc2_derivation_path_t *output);

bool bc2_derivation_format(
    const bc2_derivation_path_t *path,
    char *output,
    size_t output_size);

bool bc2_derivation_is_allowed(
    const bc2_wallet_policy_t *policy,
    const bc2_derivation_path_t *path);

bool bc2_derivation_is_hardened(uint32_t component);

uint32_t bc2_derivation_unharden(uint32_t component);

#endif