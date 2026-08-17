#include "bc2_hal.h"

#include <string.h>

static bool valid_key(const char *key) {
    return key != NULL && key[0] != '\0' && strlen(key) < BC2_HAL_STORAGE_KEY_MAX;
}

bool bc2_hal_is_complete(const bc2_hal_t *hal) {
    return hal != NULL && hal->display_present != NULL && hal->button_poll != NULL &&
           hal->time_now_ms != NULL && hal->random_fill != NULL && hal->storage_read != NULL &&
           hal->storage_write != NULL && hal->storage_remove != NULL && hal->usb_send != NULL &&
           hal->usb_receive != NULL;
}

bc2_hal_result_t bc2_hal_present(const bc2_hal_t *hal, const bc2_display_frame_t *frame) {
    if (hal == NULL || frame == NULL) return BC2_HAL_ERROR_ARGUMENT;
    if (hal->display_present == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->display_present(hal->context, frame);
}

bc2_hal_result_t bc2_hal_poll_button(const bc2_hal_t *hal, bc2_button_event_t *event) {
    if (hal == NULL || event == NULL) return BC2_HAL_ERROR_ARGUMENT;
    if (hal->button_poll == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->button_poll(hal->context, event);
}

uint64_t bc2_hal_now_ms(const bc2_hal_t *hal) {
    if (hal == NULL || hal->time_now_ms == NULL) return 0U;
    return hal->time_now_ms(hal->context);
}

bc2_hal_result_t bc2_hal_random(const bc2_hal_t *hal, uint8_t *output, size_t output_size) {
    if (hal == NULL || output == NULL || output_size == 0U) return BC2_HAL_ERROR_ARGUMENT;
    if (hal->random_fill == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->random_fill(hal->context, output, output_size);
}

bc2_hal_result_t bc2_hal_storage_read(const bc2_hal_t *hal, const char *key, uint8_t *output,
                                      size_t output_capacity, size_t *output_size) {
    if (hal == NULL || !valid_key(key) || output == NULL || output_size == NULL || output_capacity == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    if (hal->storage_read == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->storage_read(hal->context, key, output, output_capacity, output_size);
}

bc2_hal_result_t bc2_hal_storage_write(const bc2_hal_t *hal, const char *key, const uint8_t *data,
                                       size_t data_size) {
    if (hal == NULL || !valid_key(key) || data == NULL || data_size == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    if (hal->storage_write == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->storage_write(hal->context, key, data, data_size);
}

bc2_hal_result_t bc2_hal_storage_remove(const bc2_hal_t *hal, const char *key) {
    if (hal == NULL || !valid_key(key)) return BC2_HAL_ERROR_ARGUMENT;
    if (hal->storage_remove == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->storage_remove(hal->context, key);
}

bc2_hal_result_t bc2_hal_usb_send(const bc2_hal_t *hal, const uint8_t *data, size_t data_size) {
    if (hal == NULL || data == NULL || data_size == 0U) return BC2_HAL_ERROR_ARGUMENT;
    if (data_size > BC2_HAL_USB_MAX_MESSAGE) return BC2_HAL_ERROR_LIMIT;
    if (hal->usb_send == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->usb_send(hal->context, data, data_size);
}

bc2_hal_result_t bc2_hal_usb_receive(const bc2_hal_t *hal, uint8_t *output,
                                     size_t output_capacity, size_t *output_size) {
    if (hal == NULL || output == NULL || output_size == NULL || output_capacity == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    if (output_capacity > BC2_HAL_USB_MAX_MESSAGE) return BC2_HAL_ERROR_LIMIT;
    if (hal->usb_receive == NULL) return BC2_HAL_ERROR_UNAVAILABLE;
    return hal->usb_receive(hal->context, output, output_capacity, output_size);
}
