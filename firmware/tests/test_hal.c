#include "bc2_device_ui.h"
#include "bc2_hal.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    bc2_display_frame_t frame;
    uint64_t now;
    uint8_t stored[32];
    size_t stored_size;
    uint8_t usb[32];
    size_t usb_size;
} fake_hal_t;

static bc2_hal_result_t present(void *ctx, const bc2_display_frame_t *frame) {
    ((fake_hal_t *)ctx)->frame = *frame;
    return BC2_HAL_OK;
}
static bc2_hal_result_t poll(void *ctx, bc2_button_event_t *event) {
    (void)ctx;
    event->button = BC2_BUTTON_CONFIRM;
    event->action = BC2_BUTTON_PRESSED;
    event->timestamp_ms = 123U;
    return BC2_HAL_OK;
}
static uint64_t now_ms(void *ctx) { return ((fake_hal_t *)ctx)->now; }
static bc2_hal_result_t random_fill(void *ctx, uint8_t *out, size_t size) {
    (void)ctx;
    for (size_t i = 0; i < size; ++i) out[i] = (uint8_t)(i + 1U);
    return BC2_HAL_OK;
}
static bc2_hal_result_t storage_read(void *ctx, const char *key, uint8_t *out, size_t cap, size_t *size) {
    fake_hal_t *fake = (fake_hal_t *)ctx;
    (void)key;
    if (fake->stored_size == 0U) return BC2_HAL_ERROR_NOT_FOUND;
    if (cap < fake->stored_size) return BC2_HAL_ERROR_LIMIT;
    memcpy(out, fake->stored, fake->stored_size);
    *size = fake->stored_size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t storage_write(void *ctx, const char *key, const uint8_t *data, size_t size) {
    fake_hal_t *fake = (fake_hal_t *)ctx;
    (void)key;
    if (size > sizeof(fake->stored)) return BC2_HAL_ERROR_LIMIT;
    memcpy(fake->stored, data, size);
    fake->stored_size = size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t storage_remove(void *ctx, const char *key) {
    fake_hal_t *fake = (fake_hal_t *)ctx;
    (void)key;
    memset(fake->stored, 0, sizeof(fake->stored));
    fake->stored_size = 0U;
    return BC2_HAL_OK;
}
static bc2_hal_result_t usb_send(void *ctx, const uint8_t *data, size_t size) {
    fake_hal_t *fake = (fake_hal_t *)ctx;
    if (size > sizeof(fake->usb)) return BC2_HAL_ERROR_LIMIT;
    memcpy(fake->usb, data, size);
    fake->usb_size = size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t usb_receive(void *ctx, uint8_t *out, size_t cap, size_t *size) {
    fake_hal_t *fake = (fake_hal_t *)ctx;
    if (cap < fake->usb_size) return BC2_HAL_ERROR_LIMIT;
    memcpy(out, fake->usb, fake->usb_size);
    *size = fake->usb_size;
    return BC2_HAL_OK;
}

int main(void) {
    fake_hal_t fake = {0};
    fake.now = 456U;
    bc2_hal_t hal = {&fake, present, poll, now_ms, random_fill, storage_read,
                     storage_write, storage_remove, usb_send, usb_receive};
    assert(bc2_hal_is_complete(&hal));
    assert(bc2_hal_now_ms(&hal) == 456U);

    assert(bc2_device_ui_render(&hal, BC2_DEVICE_SCREEN_RECEIVE_REVIEW,
                                "bc21qtestaddress", NULL) == BC2_HAL_OK);
    assert(strcmp(fake.frame.title, "ADRESSE PRUEFEN") == 0);
    assert(strcmp(fake.frame.body, "bc21qtestaddress") == 0);
    assert(fake.frame.require_full_refresh);

    assert(bc2_device_ui_render_pin(&hal, 9U, 3U) == BC2_HAL_OK);
    assert(strcmp(fake.frame.title, "PIN EINGEBEN") == 0);
    assert(strcmp(fake.frame.body, "9:3") == 0);

    uint8_t random[4] = {0};
    assert(bc2_hal_random(&hal, random, sizeof(random)) == BC2_HAL_OK);
    assert(random[0] == 1U && random[3] == 4U);

    const uint8_t value[] = {9U, 8U, 7U};
    assert(bc2_hal_storage_write(&hal, "settings", value, sizeof(value)) == BC2_HAL_OK);
    uint8_t readback[8] = {0};
    size_t read_size = 0U;
    assert(bc2_hal_storage_read(&hal, "settings", readback, sizeof(readback), &read_size) == BC2_HAL_OK);
    assert(read_size == sizeof(value) && memcmp(value, readback, sizeof(value)) == 0);
    assert(bc2_hal_storage_remove(&hal, "settings") == BC2_HAL_OK);

    assert(bc2_hal_usb_send(&hal, value, sizeof(value)) == BC2_HAL_OK);
    memset(readback, 0, sizeof(readback));
    read_size = 0U;
    assert(bc2_hal_usb_receive(&hal, readback, sizeof(readback), &read_size) == BC2_HAL_OK);
    assert(read_size == sizeof(value) && memcmp(value, readback, sizeof(value)) == 0);

    bc2_button_event_t event = {0};
    assert(bc2_hal_poll_button(&hal, &event) == BC2_HAL_OK);
    assert(event.button == BC2_BUTTON_CONFIRM && event.timestamp_ms == 123U);
    return 0;
}
