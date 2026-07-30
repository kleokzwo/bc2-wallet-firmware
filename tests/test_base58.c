#include "bc2_base58.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_empty_input_encode(void)
{
  char output[2];

  const bool result = bc2_base58_encode(
      NULL,
      0U,
      output,
      sizeof(output));

  assert(result);
  assert(strcmp(output, "") == 0);
}

static void test_empty_input_decode(void)
{
  size_t output_length = 123U;

  const bool result = bc2_base58_decode(
      NULL,
      0U,
      NULL,
      0U,
      &output_length);

  assert(result);
  assert(output_length == 0U);
}

static void test_single_zero(void)
{
  static const uint8_t input[] = {
      0x00U};

  uint8_t decoded[8];
  size_t decoded_length = 0U;
  char encoded[8];

  assert(
      bc2_base58_encode(
          input,
          sizeof(input),
          encoded,
          sizeof(encoded)));

  assert(strcmp(encoded, "1") == 0);

  assert(
      bc2_base58_decode(
          encoded,
          strlen(encoded),
          decoded,
          sizeof(decoded),
          &decoded_length));

  assert(decoded_length == sizeof(input));
  assert(memcmp(decoded, input, sizeof(input)) == 0);
}

static void test_multiple_leading_zeros(void)
{
  static const uint8_t input[] = {
      0x00U,
      0x00U,
      0x01U};

  uint8_t decoded[16];
  size_t decoded_length = 0U;
  char encoded[16];

  assert(
      bc2_base58_encode(
          input,
          sizeof(input),
          encoded,
          sizeof(encoded)));

  assert(strcmp(encoded, "112") == 0);

  assert(
      bc2_base58_decode(
          encoded,
          strlen(encoded),
          decoded,
          sizeof(decoded),
          &decoded_length));

  assert(decoded_length == sizeof(input));
  assert(memcmp(decoded, input, sizeof(input)) == 0);
}

static void test_hello_world(void)
{
  static const uint8_t input[] = {
      0x48U,
      0x65U,
      0x6cU,
      0x6cU,
      0x6fU,
      0x20U,
      0x57U,
      0x6fU,
      0x72U,
      0x6cU,
      0x64U};

  static const char expected[] =
      "JxF12TrwUP45BMd";

  uint8_t decoded[32];
  size_t decoded_length = 0U;
  char encoded[32];

  assert(
      bc2_base58_encode(
          input,
          sizeof(input),
          encoded,
          sizeof(encoded)));

  assert(strcmp(encoded, expected) == 0);

  assert(
      bc2_base58_decode(
          expected,
          strlen(expected),
          decoded,
          sizeof(decoded),
          &decoded_length));

  assert(decoded_length == sizeof(input));
  assert(memcmp(decoded, input, sizeof(input)) == 0);
}

static void test_bitcoin_address_payload(void)
{
  static const uint8_t input[] = {
      0x00U,
      0x62U,
      0xe9U,
      0x07U,
      0xb1U,
      0x5cU,
      0xbfU,
      0x27U,
      0xd5U,
      0x42U,
      0x53U,
      0x99U,
      0xebU,
      0xf6U,
      0xf0U,
      0xfbU,
      0x50U,
      0xebU,
      0xb8U,
      0x8fU,
      0x18U,
      0xc2U,
      0x9bU,
      0x7dU,
      0x93U};

  static const char expected[] =
      "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa";

  uint8_t decoded[32];
  size_t decoded_length = 0U;
  char encoded[64];

  assert(
      bc2_base58_encode(
          input,
          sizeof(input),
          encoded,
          sizeof(encoded)));

  assert(strcmp(encoded, expected) == 0);

  assert(
      bc2_base58_decode(
          expected,
          strlen(expected),
          decoded,
          sizeof(decoded),
          &decoded_length));

  assert(decoded_length == sizeof(input));
  assert(memcmp(decoded, input, sizeof(input)) == 0);
}

static void test_invalid_character_zero(void)
{
  static const char input[] = "10";

  uint8_t output[16];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_invalid_character_uppercase_o(void)
{
  static const char input[] = "1O";

  uint8_t output[16];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_invalid_character_uppercase_i(void)
{
  static const char input[] = "1I";

  uint8_t output[16];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_invalid_character_lowercase_l(void)
{
  static const char input[] = "1l";

  uint8_t output[16];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_encode_output_buffer_too_small(void)
{
  static const uint8_t input[] = {
      0x01U,
      0x02U,
      0x03U};

  char output[2] = {
      'x',
      '\0'};

  const bool result = bc2_base58_encode(
      input,
      sizeof(input),
      output,
      sizeof(output));

  assert(!result);
  assert(output[0] == '\0');
}

static void test_decode_output_buffer_too_small(void)
{
  static const char input[] = "JxF12TrwUP45BMd";

  uint8_t output[4];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_null_encode_input(void)
{
  char output[16];

  const bool result = bc2_base58_encode(
      NULL,
      1U,
      output,
      sizeof(output));

  assert(!result);
  assert(output[0] == '\0');
}

static void test_null_encode_output(void)
{
  static const uint8_t input[] = {
      0x01U};

  const bool result = bc2_base58_encode(
      input,
      sizeof(input),
      NULL,
      0U);

  assert(!result);
}

static void test_null_decode_input(void)
{
  uint8_t output[16];
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      NULL,
      1U,
      output,
      sizeof(output),
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_null_decode_output(void)
{
  static const char input[] = "2";
  size_t output_length = 99U;

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      NULL,
      0U,
      &output_length);

  assert(!result);
  assert(output_length == 0U);
}

static void test_null_output_length(void)
{
  static const char input[] = "2";
  uint8_t output[16];

  const bool result = bc2_base58_decode(
      input,
      strlen(input),
      output,
      sizeof(output),
      NULL);

  assert(!result);
}

static void test_size_helpers(void)
{
  assert(bc2_base58_encoded_size(0U) >= 1U);
  assert(bc2_base58_encoded_size(20U) >= 29U);
  assert(bc2_base58_encoded_size(25U) >= 36U);

  assert(bc2_base58_decoded_size(0U) >= 1U);
  assert(bc2_base58_decoded_size(34U) >= 25U);
}

int main(void)
{
  test_empty_input_encode();
  test_empty_input_decode();
  test_single_zero();
  test_multiple_leading_zeros();
  test_hello_world();
  test_bitcoin_address_payload();
  test_invalid_character_zero();
  test_invalid_character_uppercase_o();
  test_invalid_character_uppercase_i();
  test_invalid_character_lowercase_l();
  test_encode_output_buffer_too_small();
  test_decode_output_buffer_too_small();
  test_null_encode_input();
  test_null_encode_output();
  test_null_decode_input();
  test_null_decode_output();
  test_null_output_length();
  test_size_helpers();

  puts("Base58 tests passed.");

  return 0;
}