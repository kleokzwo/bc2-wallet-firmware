#include "bc2_sha256.h"

#include <string.h>

#define ROTATE_RIGHT(value, bits) \
  (((value) >> (bits)) | ((value) << (32U - (bits))))

#define CHOICE(x, y, z) \
  (((x) & (y)) ^ (~(x) & (z)))

#define MAJORITY(x, y, z) \
  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define BIG_SIGMA_0(x)      \
  (ROTATE_RIGHT((x), 2U) ^  \
   ROTATE_RIGHT((x), 13U) ^ \
   ROTATE_RIGHT((x), 22U))

#define BIG_SIGMA_1(x)      \
  (ROTATE_RIGHT((x), 6U) ^  \
   ROTATE_RIGHT((x), 11U) ^ \
   ROTATE_RIGHT((x), 25U))

#define SMALL_SIGMA_0(x)    \
  (ROTATE_RIGHT((x), 7U) ^  \
   ROTATE_RIGHT((x), 18U) ^ \
   ((x) >> 3U))

#define SMALL_SIGMA_1(x)    \
  (ROTATE_RIGHT((x), 17U) ^ \
   ROTATE_RIGHT((x), 19U) ^ \
   ((x) >> 10U))

static const uint32_t ROUND_CONSTANTS[64] = {
    0x428A2F98U, 0x71374491U, 0xB5C0FBCFU, 0xE9B5DBA5U,
    0x3956C25BU, 0x59F111F1U, 0x923F82A4U, 0xAB1C5ED5U,
    0xD807AA98U, 0x12835B01U, 0x243185BEU, 0x550C7DC3U,
    0x72BE5D74U, 0x80DEB1FEU, 0x9BDC06A7U, 0xC19BF174U,
    0xE49B69C1U, 0xEFBE4786U, 0x0FC19DC6U, 0x240CA1CCU,
    0x2DE92C6FU, 0x4A7484AAU, 0x5CB0A9DCU, 0x76F988DAU,
    0x983E5152U, 0xA831C66DU, 0xB00327C8U, 0xBF597FC7U,
    0xC6E00BF3U, 0xD5A79147U, 0x06CA6351U, 0x14292967U,
    0x27B70A85U, 0x2E1B2138U, 0x4D2C6DFCU, 0x53380D13U,
    0x650A7354U, 0x766A0ABBU, 0x81C2C92EU, 0x92722C85U,
    0xA2BFE8A1U, 0xA81A664BU, 0xC24B8B70U, 0xC76C51A3U,
    0xD192E819U, 0xD6990624U, 0xF40E3585U, 0x106AA070U,
    0x19A4C116U, 0x1E376C08U, 0x2748774CU, 0x34B0BCB5U,
    0x391C0CB3U, 0x4ED8AA4AU, 0x5B9CCA4FU, 0x682E6FF3U,
    0x748F82EEU, 0x78A5636FU, 0x84C87814U, 0x8CC70208U,
    0x90BEFFFAU, 0xA4506CEBU, 0xBEF9A3F7U, 0xC67178F2U};

static uint32_t read_big_endian_u32(const uint8_t *input)
{
  return ((uint32_t)input[0] << 24U) |
         ((uint32_t)input[1] << 16U) |
         ((uint32_t)input[2] << 8U) |
         ((uint32_t)input[3]);
}

static void write_big_endian_u32(
    uint8_t *output,
    const uint32_t value)
{
  output[0] = (uint8_t)(value >> 24U);
  output[1] = (uint8_t)(value >> 16U);
  output[2] = (uint8_t)(value >> 8U);
  output[3] = (uint8_t)value;
}

static void write_big_endian_u64(
    uint8_t *output,
    const uint64_t value)
{
  output[0] = (uint8_t)(value >> 56U);
  output[1] = (uint8_t)(value >> 48U);
  output[2] = (uint8_t)(value >> 40U);
  output[3] = (uint8_t)(value >> 32U);
  output[4] = (uint8_t)(value >> 24U);
  output[5] = (uint8_t)(value >> 16U);
  output[6] = (uint8_t)(value >> 8U);
  output[7] = (uint8_t)value;
}

static void bc2_sha256_transform(
    bc2_sha256_context_t *context,
    const uint8_t block[BC2_SHA256_BLOCK_SIZE])
{
  uint32_t schedule[64];

  for (size_t index = 0U; index < 16U; index++)
  {
    schedule[index] =
        read_big_endian_u32(block + (index * 4U));
  }

  for (size_t index = 16U; index < 64U; index++)
  {
    schedule[index] =
        SMALL_SIGMA_1(schedule[index - 2U]) +
        schedule[index - 7U] +
        SMALL_SIGMA_0(schedule[index - 15U]) +
        schedule[index - 16U];
  }

  uint32_t a = context->state[0];
  uint32_t b = context->state[1];
  uint32_t c = context->state[2];
  uint32_t d = context->state[3];
  uint32_t e = context->state[4];
  uint32_t f = context->state[5];
  uint32_t g = context->state[6];
  uint32_t h = context->state[7];

  for (size_t index = 0U; index < 64U; index++)
  {
    const uint32_t temporary_1 =
        h +
        BIG_SIGMA_1(e) +
        CHOICE(e, f, g) +
        ROUND_CONSTANTS[index] +
        schedule[index];

    const uint32_t temporary_2 =
        BIG_SIGMA_0(a) +
        MAJORITY(a, b, c);

    h = g;
    g = f;
    f = e;
    e = d + temporary_1;
    d = c;
    c = b;
    b = a;
    a = temporary_1 + temporary_2;
  }

  context->state[0] += a;
  context->state[1] += b;
  context->state[2] += c;
  context->state[3] += d;
  context->state[4] += e;
  context->state[5] += f;
  context->state[6] += g;
  context->state[7] += h;
}

void bc2_sha256_init(bc2_sha256_context_t *context)
{
  if (context == NULL)
  {
    return;
  }

  context->state[0] = 0x6A09E667U;
  context->state[1] = 0xBB67AE85U;
  context->state[2] = 0x3C6EF372U;
  context->state[3] = 0xA54FF53AU;
  context->state[4] = 0x510E527FU;
  context->state[5] = 0x9B05688CU;
  context->state[6] = 0x1F83D9ABU;
  context->state[7] = 0x5BE0CD19U;

  context->total_length = 0U;
  context->buffer_length = 0U;

  memset(context->buffer, 0, sizeof(context->buffer));
}

void bc2_sha256_update(
    bc2_sha256_context_t *context,
    const uint8_t *data,
    const size_t data_length)
{
  if (context == NULL ||
      (data == NULL && data_length != 0U))
  {
    return;
  }

  size_t offset = 0U;

  context->total_length += data_length;

  if (context->buffer_length > 0U)
  {
    const size_t available =
        BC2_SHA256_BLOCK_SIZE - context->buffer_length;

    const size_t amount =
        data_length < available
            ? data_length
            : available;

    memcpy(
        context->buffer + context->buffer_length,
        data,
        amount);

    context->buffer_length += amount;
    offset += amount;

    if (context->buffer_length == BC2_SHA256_BLOCK_SIZE)
    {
      bc2_sha256_transform(
          context,
          context->buffer);

      context->buffer_length = 0U;
    }
  }

  while (
      data_length - offset >= BC2_SHA256_BLOCK_SIZE)
  {
    bc2_sha256_transform(
        context,
        data + offset);

    offset += BC2_SHA256_BLOCK_SIZE;
  }

  if (offset < data_length)
  {
    const size_t remaining =
        data_length - offset;

    memcpy(
        context->buffer,
        data + offset,
        remaining);

    context->buffer_length = remaining;
  }
}

void bc2_sha256_final(
    bc2_sha256_context_t *context,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE])
{
  if (context == NULL || digest == NULL)
  {
    return;
  }

  const uint64_t total_bits =
      context->total_length * 8U;

  context->buffer[context->buffer_length++] = 0x80U;

  if (context->buffer_length > 56U)
  {
    while (
        context->buffer_length < BC2_SHA256_BLOCK_SIZE)
    {
      context->buffer[context->buffer_length++] = 0U;
    }

    bc2_sha256_transform(
        context,
        context->buffer);

    context->buffer_length = 0U;
  }

  while (context->buffer_length < 56U)
  {
    context->buffer[context->buffer_length++] = 0U;
  }

  write_big_endian_u64(
      context->buffer + 56U,
      total_bits);

  bc2_sha256_transform(
      context,
      context->buffer);

  for (size_t index = 0U; index < 8U; index++)
  {
    write_big_endian_u32(
        digest + (index * 4U),
        context->state[index]);
  }

  memset(context, 0, sizeof(*context));
}

void bc2_sha256(
    const uint8_t *data,
    const size_t data_length,
    uint8_t digest[BC2_SHA256_DIGEST_SIZE])
{
  bc2_sha256_context_t context;

  bc2_sha256_init(&context);
  bc2_sha256_update(&context, data, data_length);
  bc2_sha256_final(&context, digest);
}