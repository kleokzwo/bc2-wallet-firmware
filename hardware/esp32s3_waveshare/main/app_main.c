#include "bc2_device_flow.h"
#include "bc2_device_service.h"
#include "bc2_device_state.h"
#include "bc2_hal.h"
#include "waveshare_bsp.h"

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

#define BC2_DEVICE_LOOP_DELAY_MS 50U

static const char *TAG = "bc2";

static void render_current_state(const bc2_hal_t *hal,
                                 const bc2_device_machine *machine) {
    char status[160];
    bc2_device_view_data_t view;

    (void)snprintf(status,
                   sizeof(status),
                   "Firmware %s\n%s\nDisplay %ux%u",
                   BC2_DEVICE_FIRMWARE_VERSION,
                   BC2_BOARD_NAME,
                   BC2_BOARD_DISPLAY_WIDTH,
                   BC2_BOARD_DISPLAY_HEIGHT);
    view.primary_text = status;
    view.secondary_text = "Hardware bring-up mode";

    if (bc2_device_flow_render(hal, machine, &view) != BC2_HAL_OK)
        ESP_LOGE(TAG, "Unable to render device state");
}

static void process_button(const bc2_hal_t *hal,
                           bc2_device_machine *machine) {
    bc2_button_event_t button_event;
    bc2_device_event device_event;
    const bc2_hal_result_t result = bc2_hal_poll_button(hal, &button_event);

    if (result == BC2_HAL_ERROR_UNAVAILABLE || result == BC2_HAL_ERROR_NOT_FOUND) return;
    if (result != BC2_HAL_OK) {
        ESP_LOGW(TAG, "Button polling failed: %d", (int)result);
        return;
    }

    if (bc2_device_flow_event_from_button(machine->state,
                                          &button_event,
                                          &device_event) == 0)
        return;
    if (bc2_device_machine_dispatch(machine,
                                    device_event,
                                    bc2_hal_now_ms(hal)) != 0)
        render_current_state(hal, machine);
}

static void process_usb(const bc2_hal_t *hal,
                        const bc2_device_machine *machine) {
    const bc2_device_identity_t identity = {
        BC2_BOARD_NAME,
        BC2_BOARD_DISPLAY_WIDTH,
        BC2_BOARD_DISPLAY_HEIGHT,
    };
    const bc2_hal_result_t result = bc2_device_service_process_usb(hal,
                                                                   machine,
                                                                   &identity);
    if (result != BC2_HAL_OK &&
        result != BC2_HAL_ERROR_NOT_FOUND &&
        result != BC2_HAL_ERROR_UNAVAILABLE)
        ESP_LOGW(TAG, "USB request failed: %d", (int)result);
}

static bool random_self_test(const bc2_hal_t *hal) {
    uint8_t random_probe[32];
    return bc2_hal_random(hal, random_probe, sizeof(random_probe)) == BC2_HAL_OK;
}

void app_main(void) {
    bc2_hal_t hal = {0};
    bc2_device_machine machine;

    ESP_ERROR_CHECK(bc2_waveshare_bsp_init(&hal));
    if (!bc2_hal_is_complete(&hal)) {
        ESP_LOGE(TAG, "HAL incomplete");
        esp_restart();
    }

    bc2_device_machine_init(&machine, 1, bc2_hal_now_ms(&hal));
    render_current_state(&hal, &machine);

    const bc2_device_event boot_result = random_self_test(&hal)
        ? BC2_DEVICE_EVENT_BOOT_COMPLETE
        : BC2_DEVICE_EVENT_FATAL_ERROR;
    (void)bc2_device_machine_dispatch(&machine,
                                      boot_result,
                                      bc2_hal_now_ms(&hal));
    render_current_state(&hal, &machine);

    ESP_LOGI(TAG,
             "v%s bring-up started; display=%s buttons=%s USB=ready Wi-Fi/BLE=off",
             BC2_DEVICE_FIRMWARE_VERSION,
             bc2_waveshare_display_ready() ? "ready" : "safety-gated",
             bc2_waveshare_buttons_ready() ? "ready" : "safety-gated");

    for (;;) {
        process_usb(&hal, &machine);
        process_button(&hal, &machine);
        if (bc2_device_machine_tick(&machine, bc2_hal_now_ms(&hal)) != 0)
            render_current_state(&hal, &machine);
        vTaskDelay(pdMS_TO_TICKS(BC2_DEVICE_LOOP_DELAY_MS));
    }
}
