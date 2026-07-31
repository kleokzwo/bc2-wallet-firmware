#ifndef BC2_DEVICE_SERVICE_H
#define BC2_DEVICE_SERVICE_H

#include "bc2_device_state.h"
#include "bc2_hal.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_DEVICE_FIRMWARE_VERSION "0.17.2"
#define BC2_DEVICE_INFO_MAX 160U

typedef struct {
    const char *board_name;
    uint16_t display_width;
    uint16_t display_height;
} bc2_device_identity_t;

bc2_hal_result_t bc2_device_service_process_usb(const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity);

#ifdef __cplusplus
}
#endif

#endif
