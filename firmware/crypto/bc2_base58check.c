#include "bc2_base58check.h"

#include "bc2_base58.h"
#include "bc2_sha256d.h"

#include <stdint.h>
#include <string.h>

enum
{
  BC2_SHA256_DIGEST_SIZE = 32,
  BC2_BASE58CHECK_MAX_DATA_SIZE =
      BC2_BASE58CHECK_MAX_PAYLOAD_SIZE +
      BC2_BASE58CHECK_CHECKSUM_SIZE
};

static bool bc2_base58check_checksum_matches(
    const uint8_t expected[BC2_BASE58CHECK_CHECKSUM_SIZE],
    const uint8_t actual[BC2_BASE58CHECK_CHECKSUM_SIZE])
{
  uint8_t difference = 0U;
  size_t index;

  /*
   * Konstanter Vergleich:
   *
   * Wir brechen nicht beim ersten unterschiedlichen Byte ab.
   * Das verhindert unnötige Timing-Unterschiede.
   */
  for (index = 0U;
       index < BC2_BASE58CHECK_CHECKSUM_SIZE;
       ++index)
  {
    difference |= (uint8_t)(expected[index] ^ actual[index]);
  }

  return difference == 0U;
}

size_t bc2_base58check_encoded_size(size_t payload_length)
{
  size_t complete_length;

  if (
      payload_length >
      SIZE_MAX - BC2_BASE58CHECK_CHECKSUM_SIZE)
  {
    return 0U;
  }

  complete_length =
      payload_length + BC2_BASE58CHECK_CHECKSUM_SIZE;

  return bc2_base58_encoded_size(complete_length);
}

bool bc2_base58check_encode(
    const uint8_t *payload,
    size_t payload_length,
    char *output,
    size_t output_size)
{
  uint8_t data[BC2_BASE58CHECK_MAX_DATA_SIZE];
  uint8_t hash[BC2_SHA256_DIGEST_SIZE];
  size_t complete_length;

  if (output == NULL || output_size == 0U)
  {
    return false;
  }

  output[0] = '\0';

  if (payload_length > BC2_BASE58CHECK_MAX_PAYLOAD_SIZE)
  {
    return false;
  }

  if (payload_length > 0U && payload == NULL)
  {
    return false;
  }

  complete_length =
      payload_length + BC2_BASE58CHECK_CHECKSUM_SIZE;

  if (payload_length > 0U)
  {
    memcpy(data, payload, payload_length);
  }

  bc2_sha256d(
      payload,
      payload_length,
      hash);

  memcpy(
      &data[payload_length],
      hash,
      BC2_BASE58CHECK_CHECKSUM_SIZE);

  if (
      !bc2_base58_encode(
          data,
          complete_length,
          output,
          output_size))
  {
    memset(data, 0, sizeof(data));
    memset(hash, 0, sizeof(hash));
    return false;
  }

  memset(data, 0, sizeof(data));
  memset(hash, 0, sizeof(hash));

  return true;
}

bool bc2_base58check_decode(
    const char *input,
    size_t input_length,
    uint8_t *payload,
    size_t payload_size,
    size_t *payload_length)
{
  uint8_t decoded[BC2_BASE58CHECK_MAX_DATA_SIZE];
  uint8_t expected_hash[BC2_SHA256_DIGEST_SIZE];
  size_t decoded_length = 0U;
  size_t decoded_payload_length;
  const uint8_t *actual_checksum;
  bool checksum_valid;

  if (payload_length == NULL)
  {
    return false;
  }

  *payload_length = 0U;

  if (input == NULL || input_length == 0U)
  {
    return false;
  }

  if (
      !bc2_base58_decode(
          input,
          input_length,
          decoded,
          sizeof(decoded),
          &decoded_length))
  {
    return false;
  }

  if (decoded_length < BC2_BASE58CHECK_CHECKSUM_SIZE)
  {
    memset(decoded, 0, sizeof(decoded));
    return false;
  }

  decoded_payload_length =
      decoded_length - BC2_BASE58CHECK_CHECKSUM_SIZE;

  if (decoded_payload_length > payload_size)
  {
    memset(decoded, 0, sizeof(decoded));
    return false;
  }

  if (decoded_payload_length > 0U && payload == NULL)
  {
    memset(decoded, 0, sizeof(decoded));
    return false;
  }

  actual_checksum = &decoded[decoded_payload_length];

  bc2_sha256d(
      decoded,
      decoded_payload_length,
      expected_hash);

  checksum_valid = bc2_base58check_checksum_matches(
      expected_hash,
      actual_checksum);

  if (!checksum_valid)
  {
    memset(decoded, 0, sizeof(decoded));
    memset(expected_hash, 0, sizeof(expected_hash));
    return false;
  }

  if (decoded_payload_length > 0U)
  {
    memcpy(
        payload,
        decoded,
        decoded_payload_length);
  }

  *payload_length = decoded_payload_length;

  memset(decoded, 0, sizeof(decoded));
  memset(expected_hash, 0, sizeof(expected_hash));

  return true;
}