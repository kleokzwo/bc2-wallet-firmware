#ifndef BC2_USB_STREAM_H
#define BC2_USB_STREAM_H

#include "bc2_usb_protocol.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_USB_FRAME_MAX (BC2_USB_HEADER_SIZE + BC2_USB_PAYLOAD_MAX)
#define BC2_USB_STREAM_BUFFER_SIZE (BC2_USB_FRAME_MAX * 2U)

typedef struct {
    uint8_t data[BC2_USB_STREAM_BUFFER_SIZE];
    size_t size;
} bc2_usb_stream_t;

void bc2_usb_stream_init(bc2_usb_stream_t *stream);
int bc2_usb_stream_push(bc2_usb_stream_t *stream,
                        const uint8_t *data,
                        size_t data_size);
int bc2_usb_stream_next(bc2_usb_stream_t *stream,
                        uint8_t *frame,
                        size_t frame_capacity,
                        size_t *frame_size);

#ifdef __cplusplus
}
#endif

#endif
