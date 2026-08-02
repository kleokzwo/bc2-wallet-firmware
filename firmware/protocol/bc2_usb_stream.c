#include "bc2_usb_stream.h"

#include <string.h>

static size_t find_magic(const uint8_t *data, size_t size) {
    if (data == NULL || size < 3U) return size;
    for (size_t i = 0U; i + 2U < size; ++i) {
        if (data[i] == BC2_USB_MAGIC_0 &&
            data[i + 1U] == BC2_USB_MAGIC_1 &&
            data[i + 2U] == BC2_USB_MAGIC_2)
            return i;
    }
    return size;
}

static void discard_prefix(bc2_usb_stream_t *stream, size_t count) {
    if (count >= stream->size) {
        stream->size = 0U;
        return;
    }
    memmove(stream->data, stream->data + count, stream->size - count);
    stream->size -= count;
}

void bc2_usb_stream_init(bc2_usb_stream_t *stream) {
    if (stream != NULL) memset(stream, 0, sizeof(*stream));
}

int bc2_usb_stream_push(bc2_usb_stream_t *stream,
                        const uint8_t *data,
                        size_t data_size) {
    if (stream == NULL || (data_size > 0U && data == NULL)) return 0;
    if (data_size > sizeof(stream->data) - stream->size) return 0;
    if (data_size > 0U) {
        memcpy(stream->data + stream->size, data, data_size);
        stream->size += data_size;
    }
    return 1;
}

int bc2_usb_stream_next(bc2_usb_stream_t *stream,
                        uint8_t *frame,
                        size_t frame_capacity,
                        size_t *frame_size) {
    if (stream == NULL || frame == NULL || frame_size == NULL) return -1;
    *frame_size = 0U;

    const size_t magic_offset = find_magic(stream->data, stream->size);
    if (magic_offset == stream->size) {
        if (stream->size > 2U) discard_prefix(stream, stream->size - 2U);
        return 0;
    }
    if (magic_offset > 0U) discard_prefix(stream, magic_offset);
    if (stream->size < BC2_USB_HEADER_SIZE) return 0;

    const uint16_t payload_size = (uint16_t)stream->data[7] |
                                  ((uint16_t)stream->data[8] << 8U);
    if (payload_size > BC2_USB_PAYLOAD_MAX) {
        discard_prefix(stream, 1U);
        return -1;
    }

    const size_t total_size = BC2_USB_HEADER_SIZE + (size_t)payload_size;
    if (stream->size < total_size) return 0;
    if (frame_capacity < total_size) return -1;

    memcpy(frame, stream->data, total_size);
    discard_prefix(stream, total_size);
    *frame_size = total_size;
    return 1;
}
