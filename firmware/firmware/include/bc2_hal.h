#ifndef BC2_HAL_H
#define BC2_HAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_HAL_DISPLAY_WIDTH 200U
#define BC2_HAL_DISPLAY_HEIGHT 200U
#define BC2_HAL_STORAGE_KEY_MAX 48U
#define BC2_HAL_USB_MAX_MESSAGE 4096U

typedef enum {
    BC2_HAL_OK = 0,
    BC2_HAL_ERROR_ARGUMENT,
    BC2_HAL_ERROR_UNAVAILABLE,
    BC2_HAL_ERROR_IO,
    BC2_HAL_ERROR_LIMIT,
    BC2_HAL_ERROR_NOT_FOUND
} bc2_hal_result_t;

typedef enum {
    BC2_BUTTON_NONE = 0,
    BC2_BUTTON_LEFT,
    BC2_BUTTON_RIGHT,
    BC2_BUTTON_CONFIRM,
    BC2_BUTTON_BACK
} bc2_button_t;

typedef enum {
    BC2_BUTTON_RELEASED = 0,
    BC2_BUTTON_PRESSED,
    BC2_BUTTON_LONG_PRESSED
} bc2_button_action_t;

typedef struct {
    bc2_button_t button;
    bc2_button_action_t action;
    uint64_t timestamp_ms;
} bc2_button_event_t;

typedef struct {
    char title[32];
    char body[256];
    char footer[64];
    bool require_full_refresh;
} bc2_display_frame_t;

typedef struct {
    void *context;
    bc2_hal_result_t (*display_present)(void *context, const bc2_display_frame_t *frame);
    bc2_hal_result_t (*button_poll)(void *context, bc2_button_event_t *event);
    uint64_t (*time_now_ms)(void *context);
    bc2_hal_result_t (*random_fill)(void *context, uint8_t *output, size_t output_size);
    bc2_hal_result_t (*storage_read)(void *context, const char *key, uint8_t *output,
                                    size_t output_capacity, size_t *output_size);
    bc2_hal_result_t (*storage_write)(void *context, const char *key, const uint8_t *data,
                                     size_t data_size);
    bc2_hal_result_t (*storage_remove)(void *context, const char *key);
    bc2_hal_result_t (*usb_send)(void *context, const uint8_t *data, size_t data_size);
    bc2_hal_result_t (*usb_receive)(void *context, uint8_t *output, size_t output_capacity,
                                   size_t *output_size);
} bc2_hal_t;

bool bc2_hal_is_complete(const bc2_hal_t *hal);
bc2_hal_result_t bc2_hal_present(const bc2_hal_t *hal, const bc2_display_frame_t *frame);
bc2_hal_result_t bc2_hal_poll_button(const bc2_hal_t *hal, bc2_button_event_t *event);
uint64_t bc2_hal_now_ms(const bc2_hal_t *hal);
bc2_hal_result_t bc2_hal_random(const bc2_hal_t *hal, uint8_t *output, size_t output_size);
bc2_hal_result_t bc2_hal_storage_read(const bc2_hal_t *hal, const char *key, uint8_t *output,
                                      size_t output_capacity, size_t *output_size);
bc2_hal_result_t bc2_hal_storage_write(const bc2_hal_t *hal, const char *key, const uint8_t *data,
                                       size_t data_size);
bc2_hal_result_t bc2_hal_storage_remove(const bc2_hal_t *hal, const char *key);
bc2_hal_result_t bc2_hal_usb_send(const bc2_hal_t *hal, const uint8_t *data, size_t data_size);
bc2_hal_result_t bc2_hal_usb_receive(const bc2_hal_t *hal, uint8_t *output,
                                     size_t output_capacity, size_t *output_size);

#ifdef __cplusplus
}
#endif

#endif
