#include "bc2_base58.h"

#include <stdint.h>
#include <string.h>

static const char BC2_BASE58_ALPHABET[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

enum
{
  BC2_BASE58_MAX_WORK_SIZE = 180
};

static int bc2_base58_character_value(char character)
{
  size_t index;

  for (index = 0U;
       index < (sizeof(BC2_BASE58_ALPHABET) - 1U);
       ++index)
  {
    if (BC2_BASE58_ALPHABET[index] == character)
    {
      return (int)index;
    }
  }

  return -1;
}

size_t bc2_base58_encoded_size(size_t input_length)
{
  /*
   * Maximale Länge:
   *
   * floor(input_length * 138 / 100) + 1 Zeichen
   * plus abschließendes '\0'.
   */
  if (input_length > (SIZE_MAX - 2U) / 138U)
  {
    return 0U;
  }

  return ((input_length * 138U) / 100U) + 2U;
}

size_t bc2_base58_decoded_size(size_t input_length)
{
  /*
   * log(58) / log(256) ist ungefähr 0,733.
   *
   * 733 / 1000 wird als sichere obere Abschätzung verwendet.
   * Zusätzlich wird ein Byte Sicherheitsreserve vorgesehen.
   */
  if (input_length > (SIZE_MAX - 1U) / 733U)
  {
    return 0U;
  }

  return ((input_length * 733U) / 1000U) + 1U;
}

bool bc2_base58_encode(
    const uint8_t *input,
    size_t input_length,
    char *output,
    size_t output_size)
{
  uint8_t digits[BC2_BASE58_MAX_WORK_SIZE];
  size_t zero_count = 0U;
  size_t input_index;
  size_t encoded_length = 0U;
  size_t output_index = 0U;
  size_t buffer_size;

  if (output == NULL || output_size == 0U)
  {
    return false;
  }

  output[0] = '\0';

  if (input_length == 0U)
  {
    return true;
  }

  if (input == NULL)
  {
    return false;
  }

  while (
      zero_count < input_length &&
      input[zero_count] == 0U)
  {
    ++zero_count;
  }

  if ((input_length - zero_count) > (SIZE_MAX - 1U) / 138U)
  {
    return false;
  }

  buffer_size =
      (((input_length - zero_count) * 138U) / 100U) + 1U;

  if (buffer_size > sizeof(digits))
  {
    return false;
  }

  memset(digits, 0, sizeof(digits));

  for (input_index = zero_count;
       input_index < input_length;
       ++input_index)
  {
    unsigned int carry = input[input_index];
    size_t digit_index = 0U;

    while (
        digit_index < encoded_length ||
        carry != 0U)
    {
      if (digit_index >= buffer_size)
      {
        return false;
      }

      carry += (unsigned int)digits[digit_index] * 256U;

      digits[digit_index] = (uint8_t)(carry % 58U);
      carry /= 58U;

      ++digit_index;
    }

    if (digit_index > encoded_length)
    {
      encoded_length = digit_index;
    }
  }

  if (zero_count > SIZE_MAX - encoded_length)
  {
    return false;
  }

  if ((zero_count + encoded_length) > SIZE_MAX - 1U)
  {
    return false;
  }

  if ((zero_count + encoded_length + 1U) > output_size)
  {
    return false;
  }

  for (input_index = 0U;
       input_index < zero_count;
       ++input_index)
  {
    output[output_index++] = '1';
  }

  while (encoded_length > 0U)
  {
    --encoded_length;

    output[output_index++] =
        BC2_BASE58_ALPHABET[digits[encoded_length]];
  }

  output[output_index] = '\0';

  return true;
}

bool bc2_base58_decode(
    const char *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length)
{
  uint8_t bytes[BC2_BASE58_MAX_WORK_SIZE];
  size_t zero_count = 0U;
  size_t decoded_length = 0U;
  size_t input_index;
  size_t output_index = 0U;
  size_t buffer_size;

  if (output_length == NULL)
  {
    return false;
  }

  *output_length = 0U;

  if (input_length == 0U)
  {
    return true;
  }

  if (input == NULL || output == NULL)
  {
    return false;
  }

  while (
      zero_count < input_length &&
      input[zero_count] == '1')
  {
    ++zero_count;
  }

  buffer_size = bc2_base58_decoded_size(
      input_length - zero_count);

  if (buffer_size == 0U || buffer_size > sizeof(bytes))
  {
    return false;
  }

  memset(bytes, 0, sizeof(bytes));

  for (input_index = zero_count;
       input_index < input_length;
       ++input_index)
  {
    const int character_value =
        bc2_base58_character_value(input[input_index]);

    unsigned int carry;
    size_t byte_index = 0U;

    if (character_value < 0)
    {
      return false;
    }

    carry = (unsigned int)character_value;

    while (
        byte_index < decoded_length ||
        carry != 0U)
    {
      if (byte_index >= buffer_size)
      {
        return false;
      }

      carry += (unsigned int)bytes[byte_index] * 58U;

      bytes[byte_index] = (uint8_t)(carry & 0xffU);
      carry >>= 8U;

      ++byte_index;
    }

    if (byte_index > decoded_length)
    {
      decoded_length = byte_index;
    }
  }

  if (zero_count > SIZE_MAX - decoded_length)
  {
    return false;
  }

  if ((zero_count + decoded_length) > output_size)
  {
    return false;
  }

  for (input_index = 0U;
       input_index < zero_count;
       ++input_index)
  {
    output[output_index++] = 0U;
  }

  while (decoded_length > 0U)
  {
    --decoded_length;
    output[output_index++] = bytes[decoded_length];
  }

  *output_length = output_index;

  return true;
}