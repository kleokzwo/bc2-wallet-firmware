#ifndef BC2_DEVICE_SERVICE_H
#define BC2_DEVICE_SERVICE_H

#include "bc2_device_state.h"
#include "bc2_hal.h"
#include "bc2_usb_stream.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_DEVICE_FIRMWARE_VERSION "0.22.0"
#define BC2_DEVICE_INFO_MAX 192U

typedef enum {
    BC2_DEVICE_CAP_USB = 1U << 0,
    BC2_DEVICE_CAP_STORAGE = 1U << 1,
    BC2_DEVICE_CAP_RANDOM = 1U << 2,
    BC2_DEVICE_CAP_DISPLAY = 1U << 3,
    BC2_DEVICE_CAP_BUTTONS = 1U << 4
} bc2_device_capability_t;

typedef struct {
    const char *board_name;
    uint16_t display_width;
    uint16_t display_height;
    uint8_t board_revision;
    uint8_t capabilities;
} bc2_device_identity_t;

typedef struct {
    bc2_usb_stream_t usb_stream;
} bc2_device_service_t;

void bc2_device_service_init(bc2_device_service_t *service);

bc2_hal_result_t bc2_device_service_process_usb(bc2_device_service_t *service,
                                                const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity);

#ifdef __cplusplus
}
#endif

#endif
