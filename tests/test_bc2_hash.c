#include "bc2_hash.h"
#include "bc2_hex.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void assert_sha256d_hex(
    const uint8_t *input,
    const size_t input_length,
    const char *expected_hex)
{
  uint8_t digest[BC2_SHA256_DIGEST_SIZE];
  char actual_hex[(BC2_SHA256_DIGEST_SIZE * 2U) + 1U];

  bc2_sha256d(
      input,
      input_length,
      digest);

  assert(
      bc2_hex_encode(
          digest,
          sizeof(digest),
          actual_hex,
          sizeof(actual_hex)));

  assert(strcmp(actual_hex, expected_hex) == 0);
}

static void test_sha256d_empty(void)
{
  assert_sha256d_hex(
      NULL,
      0U,
      "5df6e0e2761359d30a8275058e299fcc"
      "0381534545f55cf43e41983f5d4c9456");
}

static void test_sha256d_abc(void)
{
  static const uint8_t input[] = {
      'a', 'b', 'c'};

  assert_sha256d_hex(
      input,
      sizeof(input),
      "4f8b42c22dd3729b519ba6f68d2da7cc"
      "5b2d606d05daed5ad5128cc03e6c6358");
}

static void test_sha256d_checksum(void)
{
  static const uint8_t input[] = {
      'a', 'b', 'c'};

  static const uint8_t expected[4] = {
      0x4F, 0x8B, 0x42, 0xC2};

  uint8_t checksum[4];

  bc2_sha256d_checksum(
      input,
      sizeof(input),
      checksum);

  assert(
      memcmp(
          checksum,
          expected,
          sizeof(expected)) == 0);
}

int main(void)
{
  test_sha256d_empty();
  test_sha256d_abc();
  test_sha256d_checksum();

  puts("All BC2 hash tests passed.");
  return 0;
}