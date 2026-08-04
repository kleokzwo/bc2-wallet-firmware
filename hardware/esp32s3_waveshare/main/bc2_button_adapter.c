#include "bc2_button_adapter.h"

#include "driver/gpio.h"
#include "esp_timer.h"

#include <stddef.h>
#include <string.h>

#define BC2_BUTTON_COUNT 2U
#define BC2_BUTTON_DEBOUNCE_MS 35U

static const gpio_num_t k_pins[BC2_BUTTON_COUNT] = {GPIO_NUM_18, GPIO_NUM_0};
static const bc2_button_t k_buttons[BC2_BUTTON_COUNT] = {BC2_BUTTON_RIGHT, BC2_BUTTON_LEFT};
static int g_previous_level[BC2_BUTTON_COUNT];
static uint64_t g_last_change_ms[BC2_BUTTON_COUNT];

esp_err_t bc2_button_adapter_init(void) {
    const gpio_config_t config = {
        .pin_bit_mask = (1ULL << GPIO_NUM_18) | (1ULL << GPIO_NUM_0),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    size_t index;
    const esp_err_t result = gpio_config(&config);
    if (result != ESP_OK) return result;
    for (index = 0U; index < BC2_BUTTON_COUNT; ++index) {
        g_previous_level[index] = gpio_get_level(k_pins[index]);
        g_last_change_ms[index] = 0U;
    }
    return ESP_OK;
}

bc2_hal_result_t bc2_button_adapter_poll(bc2_button_event_t *event) {
    const uint64_t now = (uint64_t)(esp_timer_get_time() / 1000);
    size_t index;
    if (event == NULL) return BC2_HAL_ERROR_ARGUMENT;
    memset(event, 0, sizeof(*event));

    for (index = 0U; index < BC2_BUTTON_COUNT; ++index) {
        const int level = gpio_get_level(k_pins[index]);
        if (level == g_previous_level[index]) continue;
        if (now - g_last_change_ms[index] < BC2_BUTTON_DEBOUNCE_MS) continue;
        g_previous_level[index] = level;
        g_last_change_ms[index] = now;
        event->button = k_buttons[index];
        event->action = level == 0 ? BC2_BUTTON_PRESSED : BC2_BUTTON_RELEASED;
        event->timestamp_ms = now;
        return BC2_HAL_OK;
    }
    return BC2_HAL_ERROR_NOT_FOUND;
}
