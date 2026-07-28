#include "bc2_hash.h"

#include <string.h>

void bc2_sha256d(
    const uint8_t *data,
    const size_t data_length,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE])
{
  uint8_t first_digest[BC2_SHA256_DIGEST_SIZE];

  if (digest == NULL)
  {
    return;
  }

  if (data == NULL && data_length != 0U)
  {
    memset(digest, 0, BC2_SHA256_DIGEST_SIZE);
    return;
  }

  bc2_sha256(
      data,
      data_length,
      first_digest);

  bc2_sha256(
      first_digest,
      sizeof(first_digest),
      digest);

  memset(
      first_digest,
      0,
      sizeof(first_digest));
}

void bc2_sha256d_checksum(
    const uint8_t *data,
    const size_t data_length,
    uint8_t checksum[4])
{
  uint8_t digest[BC2_SHA256_DIGEST_SIZE];

  if (checksum == NULL)
  {
    return;
  }

  bc2_sha256d(
      data,
      data_length,
      digest);

  memcpy(
      checksum,
      digest,
      4U);

  memset(
      digest,
      0,
      sizeof(digest));
}