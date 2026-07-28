#include "bc2_derivation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const bc2_wallet_policy_t BITCOIN_COMPATIBLE_POLICY = {
    .name = "BC2 Bitcoin-compatible Native SegWit",
    .type = BC2_POLICY_BITCOIN_COMPATIBLE,
    .script_type = BC2_SCRIPT_NATIVE_SEGWIT,
    .purpose = 84U,
    .coin_type = 0U,
    .account = 0U};

static const bc2_wallet_policy_t BC2_NATIVE_POLICY = {
    .name = "BC2 Native",
    .type = BC2_POLICY_NATIVE,
    .script_type = BC2_SCRIPT_NATIVE_SEGWIT,
    .purpose = 84U,
    .coin_type = 4541509U,
    .account = 0U};

static const bc2_wallet_policy_t BC2_LEGACY_POLICY = {
    .name = "BC2 Bitcoin-compatible Legacy",
    .type = BC2_POLICY_LEGACY,
    .script_type = BC2_SCRIPT_LEGACY_P2PKH,
    .purpose = 44U,
    .coin_type = 0U,
    .account = 0U};

static bool append_component(
    bc2_derivation_path_t *path,
    uint32_t component)
{
  if (path == NULL ||
      path->depth >= BC2_DERIVATION_MAX_DEPTH)
  {
    return false;
  }

  path->components[path->depth] = component;
  path->depth++;

  return true;
}

static bool append_hardened(
    bc2_derivation_path_t *path,
    uint32_t component)
{
  if (component >= BC2_DERIVATION_HARDENED)
  {
    return false;
  }

  return append_component(
      path,
      component | BC2_DERIVATION_HARDENED);
}

const bc2_wallet_policy_t *
bc2_wallet_policy_get(const bc2_wallet_policy_type_t type)
{
  switch (type)
  {
  case BC2_POLICY_BITCOIN_COMPATIBLE:
    return &BITCOIN_COMPATIBLE_POLICY;

  case BC2_POLICY_NATIVE:
    return &BC2_NATIVE_POLICY;

  case BC2_POLICY_LEGACY:
    return &BC2_LEGACY_POLICY;

  default:
    return NULL;
  }
}

bool bc2_derivation_account_path(
    const bc2_wallet_policy_t *policy,
    bc2_derivation_path_t *output)
{
  if (policy == NULL || output == NULL)
  {
    return false;
  }

  output->depth = 0U;

  return append_hardened(output, policy->purpose) &&
         append_hardened(output, policy->coin_type) &&
         append_hardened(output, policy->account);
}

bool bc2_derivation_address_path(
    const bc2_wallet_policy_t *policy,
    const uint32_t change,
    const uint32_t address_index,
    bc2_derivation_path_t *output)
{
  if (policy == NULL ||
      output == NULL ||
      change > 1U ||
      address_index >= BC2_DERIVATION_HARDENED)
  {
    return false;
  }

  if (!bc2_derivation_account_path(policy, output))
  {
    return false;
  }

  return append_component(output, change) &&
         append_component(output, address_index);
}

bool bc2_derivation_parse(
    const char *text,
    bc2_derivation_path_t *output)
{
  char buffer[256];
  char *token;
  char *save_pointer = NULL;

  if (text == NULL || output == NULL)
  {
    return false;
  }

  const size_t text_length = strlen(text);

  if (text_length < 1U ||
      text_length >= sizeof(buffer))
  {
    return false;
  }

  memcpy(buffer, text, text_length + 1U);
  output->depth = 0U;

  token = strtok_r(buffer, "/", &save_pointer);

  if (token == NULL || strcmp(token, "m") != 0)
  {
    return false;
  }

  while ((token = strtok_r(NULL, "/", &save_pointer)) != NULL)
  {
    bool hardened = false;
    char *end_pointer = NULL;
    unsigned long value;

    const size_t token_length = strlen(token);

    if (token_length == 0U)
    {
      return false;
    }

    if (token[token_length - 1U] == '\'' ||
        token[token_length - 1U] == 'h' ||
        token[token_length - 1U] == 'H')
    {
      hardened = true;
      token[token_length - 1U] = '\0';
    }

    if (token[0] == '\0')
    {
      return false;
    }

    errno = 0;
    value = strtoul(token, &end_pointer, 10);

    if (errno != 0 ||
        end_pointer == token ||
        *end_pointer != '\0' ||
        value >= BC2_DERIVATION_HARDENED)
    {
      return false;
    }

    uint32_t component = (uint32_t)value;

    if (hardened)
    {
      component |= BC2_DERIVATION_HARDENED;
    }

    if (!append_component(output, component))
    {
      return false;
    }
  }

  return true;
}

bool bc2_derivation_format(
    const bc2_derivation_path_t *path,
    char *output,
    const size_t output_size)
{
  size_t offset = 0U;
  int written;

  if (path == NULL ||
      output == NULL ||
      output_size < 2U ||
      path->depth > BC2_DERIVATION_MAX_DEPTH)
  {
    return false;
  }

  written = snprintf(output, output_size, "m");

  if (written < 0 || (size_t)written >= output_size)
  {
    return false;
  }

  offset = (size_t)written;

  for (size_t i = 0U; i < path->depth; i++)
  {
    const uint32_t component = path->components[i];
    const uint32_t value =
        bc2_derivation_unharden(component);

    written = snprintf(
        output + offset,
        output_size - offset,
        bc2_derivation_is_hardened(component)
            ? "/%lu'"
            : "/%lu",
        (unsigned long)value);

    if (written < 0 ||
        (size_t)written >= output_size - offset)
    {
      return false;
    }

    offset += (size_t)written;
  }

  return true;
}

bool bc2_derivation_is_allowed(
    const bc2_wallet_policy_t *policy,
    const bc2_derivation_path_t *path)
{
  if (policy == NULL ||
      path == NULL ||
      path->depth != 5U)
  {
    return false;
  }

  const uint32_t expected_purpose =
      policy->purpose | BC2_DERIVATION_HARDENED;

  const uint32_t expected_coin_type =
      policy->coin_type | BC2_DERIVATION_HARDENED;

  const uint32_t expected_account =
      policy->account | BC2_DERIVATION_HARDENED;

  if (path->components[0] != expected_purpose ||
      path->components[1] != expected_coin_type ||
      path->components[2] != expected_account)
  {
    return false;
  }

  if (bc2_derivation_is_hardened(path->components[3]) ||
      bc2_derivation_is_hardened(path->components[4]))
  {
    return false;
  }

  const uint32_t change = path->components[3];

  if (change > 1U)
  {
    return false;
  }

  return true;
}

bool bc2_derivation_is_hardened(const uint32_t component)
{
  return (
             component & BC2_DERIVATION_HARDENED) != 0U;
}

uint32_t bc2_derivation_unharden(const uint32_t component)
{
  return component & ~BC2_DERIVATION_HARDENED;
}