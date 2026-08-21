#include "bc2_pin_security.h"

#include <assert.h>
#include <string.h>

typedef struct {
    uint8_t record[128];
    size_t record_size;
} fake_storage_t;

static uint64_t now_ms(void *context) { (void)context; return 0U; }
static bc2_hal_result_t random_fill(void *context, uint8_t *output, size_t size) {
    size_t index;
    (void)context;
    for (index = 0U; index < size; ++index) output[index] = (uint8_t)(index + 7U);
    return BC2_HAL_OK;
}
static bc2_hal_result_t read_value(void *context, const char *key, uint8_t *output,
                                   size_t capacity, size_t *size) {
    fake_storage_t *storage = (fake_storage_t *)context;
    (void)key;
    if (storage->record_size == 0U) return BC2_HAL_ERROR_NOT_FOUND;
    if (capacity < storage->record_size) return BC2_HAL_ERROR_LIMIT;
    memcpy(output, storage->record, storage->record_size);
    *size = storage->record_size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t write_value(void *context, const char *key, const uint8_t *data,
                                    size_t size) {
    fake_storage_t *storage = (fake_storage_t *)context;
    (void)key;
    if (size > sizeof(storage->record)) return BC2_HAL_ERROR_LIMIT;
    memcpy(storage->record, data, size);
    storage->record_size = size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t remove_value(void *context, const char *key) {
    fake_storage_t *storage = (fake_storage_t *)context;
    (void)key;
    memset(storage, 0, sizeof(*storage));
    return BC2_HAL_OK;
}

static int contains_pin(const uint8_t *data, size_t size, const char *pin) {
    size_t index;
    for (index = 0U; index + BC2_SECURITY_PIN_LENGTH <= size; ++index)
        if (memcmp(data + index, pin, BC2_SECURITY_PIN_LENGTH) == 0) return 1;
    return 0;
}

int main(void) {
    fake_storage_t storage = {0};
    bc2_hal_t hal = {0};
    bc2_pin_security_t security;
    hal.context = &storage;
    hal.time_now_ms = now_ms;
    hal.random_fill = random_fill;
    hal.storage_read = read_value;
    hal.storage_write = write_value;
    hal.storage_remove = remove_value;

    assert(bc2_pin_security_load(&security, &hal, 0U) ==
           BC2_PIN_SECURITY_NOT_CONFIGURED);
    assert(bc2_pin_security_create(&security, &hal, "2468") == BC2_PIN_SECURITY_OK);
    assert(storage.record_size > BC2_SECURITY_PIN_LENGTH);
    assert(!contains_pin(storage.record, storage.record_size, "2468"));
    assert(bc2_pin_security_verify(&security, &hal, "2468", 0U) == BC2_PIN_SECURITY_OK);
    assert(bc2_pin_security_verify(&security, &hal, "1111", 100U) == BC2_PIN_SECURITY_INVALID);
    assert(!bc2_pin_security_is_locked_down(&security));
    assert(bc2_pin_security_verify(&security, &hal, "2222", 200U) == BC2_PIN_SECURITY_INVALID);
    assert(!bc2_pin_security_is_locked_down(&security));
    assert(bc2_pin_security_verify(&security, &hal, "3333", 300U) == BC2_PIN_SECURITY_LOCKED);
    assert(bc2_pin_security_is_locked_down(&security));
    assert(bc2_pin_security_verify(&security, &hal, "2468", 400U) == BC2_PIN_SECURITY_LOCKED);

    /* Lockdown survives a reboot because the failure count is persisted. */
    bc2_pin_security_t after_reboot;
    assert(bc2_pin_security_load(&after_reboot, &hal, 500U) == BC2_PIN_SECURITY_OK);
    assert(bc2_pin_security_is_locked_down(&after_reboot));
    assert(bc2_pin_security_verify(&after_reboot, &hal, "2468", 500U) == BC2_PIN_SECURITY_LOCKED);

    /* Recovery/new PIN creation is the explicit way out. */
    assert(bc2_pin_security_create(&after_reboot, &hal, "1357") == BC2_PIN_SECURITY_OK);
    assert(!bc2_pin_security_is_locked_down(&after_reboot));
    assert(bc2_pin_security_verify(&after_reboot, &hal, "1357", 600U) == BC2_PIN_SECURITY_OK);

    assert(bc2_authorization_required_pin(BC2_AUTH_TRANSACTION) == BC2_AUTH_DEVICE_PIN);
    assert(bc2_authorization_required_pin(BC2_AUTH_RECEIVE_ADDRESS) == BC2_AUTH_DEVICE_PIN);
    assert(bc2_authorization_required_pin(BC2_AUTH_NEW_WALLET) == BC2_AUTH_ROOT_PIN);
    return 0;
}
