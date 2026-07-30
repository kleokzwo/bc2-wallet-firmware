#ifndef BC2_DEVICE_UI_H
#define BC2_DEVICE_UI_H

#include "bc2_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BC2_DEVICE_SCREEN_BOOT = 0,
    BC2_DEVICE_SCREEN_LOCKED,
    BC2_DEVICE_SCREEN_DASHBOARD,
    BC2_DEVICE_SCREEN_RECEIVE_REVIEW,
    BC2_DEVICE_SCREEN_TRANSACTION_SUMMARY,
    BC2_DEVICE_SCREEN_ERROR
} bc2_device_screen_t;

bc2_hal_result_t bc2_device_ui_render(const bc2_hal_t *hal, bc2_device_screen_t screen,
                                      const char *primary_text, const char *secondary_text);

#ifdef __cplusplus
}
#endif

#endif
