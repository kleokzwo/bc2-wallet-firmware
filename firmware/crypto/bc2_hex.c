#include "bc2_hex.h"

#include <limits.h>
#include <string.h>

static int hex_character_value(const char character)
{
  if (character >= '0' && character <= '9')
  {
    return character - '0';
  }

  if (character >= 'a' && character <= 'f')
  {
    return character - 'a' + 10;
  }

  if (character >= 'A' && character <= 'F')
  {
    return character - 'A' + 10;
  }

  return -1;
}

size_t bc2_hex_encoded_size(const size_t input_length)
{
  if (input_length > (SIZE_MAX - 1U) / 2U)
  {
    return 0U;
  }

  return (input_length * 2U) + 1U;
}

bool bc2_hex_encode(
    const uint8_t *input,
    const size_t input_length,
    char *output,
    const size_t output_size)
{
  static const char HEX_ALPHABET[] =
      "0123456789abcdef";

  const size_t required_size =
      bc2_hex_encoded_size(input_length);

  if (output == NULL ||
      required_size == 0U ||
      output_size < required_size)
  {
    return false;
  }

  if (input == NULL && input_length != 0U)
  {
    return false;
  }

  for (size_t index = 0U;
       index < input_length;
       index++)
  {
    output[index * 2U] =
        HEX_ALPHABET[input[index] >> 4U];

    output[(index * 2U) + 1U] =
        HEX_ALPHABET[input[index] & 0x0FU];
  }

  output[input_length * 2U] = '\0';

  return true;
}

bool bc2_hex_decode(
    const char *input,
    uint8_t *output,
    const size_t output_size,
    size_t *decoded_length)
{
  if (input == NULL ||
      output == NULL ||
      decoded_length == NULL)
  {
    return false;
  }

  const size_t input_length = strlen(input);

  if ((input_length % 2U) != 0U)
  {
    return false;
  }

  const size_t required_output_size =
      input_length / 2U;

  if (output_size < required_output_size)
  {
    return false;
  }

  for (size_t index = 0U;
       index < required_output_size;
       index++)
  {
    const int high =
        hex_character_value(input[index * 2U]);

    const int low =
        hex_character_value(
            input[(index * 2U) + 1U]);

    if (high < 0 || low < 0)
    {
      return false;
    }

    output[index] =
        (uint8_t)((high << 4) | low);
  }

  *decoded_length = required_output_size;

  return true;
}