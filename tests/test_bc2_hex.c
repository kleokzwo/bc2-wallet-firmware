#include "bc2_hex.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_encode(void)
{
  static const uint8_t input[] = {
      0x00,
      0x01,
      0x7F,
      0x80,
      0xAB,
      0xFF};

  char output[13];

  assert(
      bc2_hex_encode(
          input,
          sizeof(input),
          output,
          sizeof(output)));

  assert(
      strcmp(output, "00017f80abff") == 0);
}

static void test_decode_lowercase(void)
{
  static const uint8_t expected[] = {
      0x00,
      0x01,
      0x7F,
      0x80,
      0xAB,
      0xFF};

  uint8_t output[sizeof(expected)];
  size_t decoded_length = 0U;

  assert(
      bc2_hex_decode(
          "00017f80abff",
          output,
          sizeof(output),
          &decoded_length));

  assert(decoded_length == sizeof(expected));

  assert(
      memcmp(
          output,
          expected,
          sizeof(expected)) == 0);
}

static void test_decode_uppercase(void)
{
  uint8_t output[3];
  size_t decoded_length = 0U;

  assert(
      bc2_hex_decode(
          "A1B2C3",
          output,
          sizeof(output),
          &decoded_length));

  assert(decoded_length == 3U);
  assert(output[0] == 0xA1U);
  assert(output[1] == 0xB2U);
  assert(output[2] == 0xC3U);
}

static void test_empty_value(void)
{
  char encoded[1];
  uint8_t decoded[1] = {0xAAU};
  size_t decoded_length = 99U;

  assert(
      bc2_hex_encode(
          NULL,
          0U,
          encoded,
          sizeof(encoded)));

  assert(strcmp(encoded, "") == 0);

  assert(
      bc2_hex_decode(
          "",
          decoded,
          sizeof(decoded),
          &decoded_length));

  assert(decoded_length == 0U);
}

static void test_invalid_hex(void)
{
  uint8_t output[4];
  size_t decoded_length = 0U;

  assert(
      !bc2_hex_decode(
          "abc",
          output,
          sizeof(output),
          &decoded_length));

  assert(
      !bc2_hex_decode(
          "gg",
          output,
          sizeof(output),
          &decoded_length));

  assert(
      !bc2_hex_decode(
          "0011",
          output,
          1U,
          &decoded_length));
}

static void test_output_too_small(void)
{
  static const uint8_t input[] = {
      0xAB,
      0xCD};

  char output[4];

  assert(
      !bc2_hex_encode(
          input,
          sizeof(input),
          output,
          sizeof(output)));
}

int main(void)
{
  test_encode();
  test_decode_lowercase();
  test_decode_uppercase();
  test_empty_value();
  test_invalid_hex();
  test_output_too_small();

  puts("All BC2 hex tests passed.");
  return 0;
}