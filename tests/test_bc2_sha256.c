#include "bc2_sha256.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void assert_digest_equals(
    const uint8_t actual[BC2_SHA256_DIGEST_SIZE],
    const uint8_t expected[BC2_SHA256_DIGEST_SIZE])
{
  assert(
      memcmp(
          actual,
          expected,
          BC2_SHA256_DIGEST_SIZE) == 0);
}

static void test_empty_message(void)
{
  static const uint8_t expected[BC2_SHA256_DIGEST_SIZE] = {
      0xE3, 0xB0, 0xC4, 0x42,
      0x98, 0xFC, 0x1C, 0x14,
      0x9A, 0xFB, 0xF4, 0xC8,
      0x99, 0x6F, 0xB9, 0x24,
      0x27, 0xAE, 0x41, 0xE4,
      0x64, 0x9B, 0x93, 0x4C,
      0xA4, 0x95, 0x99, 0x1B,
      0x78, 0x52, 0xB8, 0x55};

  uint8_t digest[BC2_SHA256_DIGEST_SIZE];

  bc2_sha256(NULL, 0U, digest);

  assert_digest_equals(digest, expected);
}

static void test_abc(void)
{
  static const uint8_t input[] = {
      'a', 'b', 'c'};

  static const uint8_t expected[BC2_SHA256_DIGEST_SIZE] = {
      0xBA, 0x78, 0x16, 0xBF,
      0x8F, 0x01, 0xCF, 0xEA,
      0x41, 0x41, 0x40, 0xDE,
      0x5D, 0xAE, 0x22, 0x23,
      0xB0, 0x03, 0x61, 0xA3,
      0x96, 0x17, 0x7A, 0x9C,
      0xB4, 0x10, 0xFF, 0x61,
      0xF2, 0x00, 0x15, 0xAD};

  uint8_t digest[BC2_SHA256_DIGEST_SIZE];

  bc2_sha256(
      input,
      sizeof(input),
      digest);

  assert_digest_equals(digest, expected);
}

static void test_long_message(void)
{
  static const uint8_t input[] =
      "abcdbcdecdefdefgefghfghighijhijk"
      "ijkljklmklmnlmnomnopnopq";

  static const uint8_t expected[BC2_SHA256_DIGEST_SIZE] = {
      0x24, 0x8D, 0x6A, 0x61,
      0xD2, 0x06, 0x38, 0xB8,
      0xE5, 0xC0, 0x26, 0x93,
      0x0C, 0x3E, 0x60, 0x39,
      0xA3, 0x3C, 0xE4, 0x59,
      0x64, 0xFF, 0x21, 0x67,
      0xF6, 0xEC, 0xED, 0xD4,
      0x19, 0xDB, 0x06, 0xC1};

  uint8_t digest[BC2_SHA256_DIGEST_SIZE];

  bc2_sha256(
      input,
      sizeof(input) - 1U,
      digest);

  assert_digest_equals(digest, expected);
}

static void test_incremental_update(void)
{
  static const uint8_t expected[BC2_SHA256_DIGEST_SIZE] = {
      0xBA, 0x78, 0x16, 0xBF,
      0x8F, 0x01, 0xCF, 0xEA,
      0x41, 0x41, 0x40, 0xDE,
      0x5D, 0xAE, 0x22, 0x23,
      0xB0, 0x03, 0x61, 0xA3,
      0x96, 0x17, 0x7A, 0x9C,
      0xB4, 0x10, 0xFF, 0x61,
      0xF2, 0x00, 0x15, 0xAD};

  bc2_sha256_context_t context;
  uint8_t digest[BC2_SHA256_DIGEST_SIZE];

  bc2_sha256_init(&context);

  bc2_sha256_update(
      &context,
      (const uint8_t *)"a",
      1U);

  bc2_sha256_update(
      &context,
      (const uint8_t *)"b",
      1U);

  bc2_sha256_update(
      &context,
      (const uint8_t *)"c",
      1U);

  bc2_sha256_final(&context, digest);

  assert_digest_equals(digest, expected);
}

int main(void)
{
  test_empty_message();
  test_abc();
  test_long_message();
  test_incremental_update();

  puts("All BC2 SHA-256 tests passed.");
  return 0;
}