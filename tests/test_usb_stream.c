#include "bc2_usb_stream.h"

#include <assert.h>
#include <string.h>

int main(void) {
    uint8_t first[32];
    uint8_t second[32];
    uint8_t output[BC2_USB_FRAME_MAX];
    size_t output_size = 0U;
    const uint8_t ping[] = {'o', 'k'};
    const size_t first_size = bc2_usb_encode(BC2_USB_CMD_PING, 1U, ping,
                                             sizeof(ping), first, sizeof(first));
    const size_t second_size = bc2_usb_encode(BC2_USB_CMD_GET_INFO, 2U, NULL,
                                              0U, second, sizeof(second));
    assert(first_size > 0U && second_size > 0U);

    bc2_usb_stream_t stream;
    bc2_usb_stream_init(&stream);

    assert(bc2_usb_stream_push(&stream, first, 4U) == 1);
    assert(bc2_usb_stream_next(&stream, output, sizeof(output), &output_size) == 0);
    assert(bc2_usb_stream_push(&stream, first + 4U, first_size - 4U) == 1);
    assert(bc2_usb_stream_next(&stream, output, sizeof(output), &output_size) == 1);
    assert(output_size == first_size);
    assert(memcmp(output, first, first_size) == 0);

    uint8_t combined[64];
    memcpy(combined, first, first_size);
    memcpy(combined + first_size, second, second_size);
    assert(bc2_usb_stream_push(&stream, combined, first_size + second_size) == 1);
    assert(bc2_usb_stream_next(&stream, output, sizeof(output), &output_size) == 1);
    assert(output_size == first_size);
    assert(bc2_usb_stream_next(&stream, output, sizeof(output), &output_size) == 1);
    assert(output_size == second_size);

    const uint8_t noise[] = {0x00U, 0xffU, 0x42U, 0x00U};
    assert(bc2_usb_stream_push(&stream, noise, sizeof(noise)) == 1);
    assert(bc2_usb_stream_push(&stream, first, first_size) == 1);
    assert(bc2_usb_stream_next(&stream, output, sizeof(output), &output_size) == 1);
    assert(output_size == first_size);
    return 0;
}
