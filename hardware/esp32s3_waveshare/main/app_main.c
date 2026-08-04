#include "bc2_device_flow.h"
#include "bc2_button_adapter.h"
#include "bc2_device_service.h"
#include "bc2_device_state.h"
#include "bc2_hal.h"
#include "bc2_navigation.h"
#include "bc2_pin_entry.h"
#include "bc2_pin_security.h"
#include "waveshare_bsp.h"

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>

#define BC2_DEVICE_LOOP_DELAY_MS 50U
#define BC2_APPLICATION_TASK_STACK_SIZE 16384U
#define BC2_APPLICATION_TASK_PRIORITY 5U
static const char *TAG = "bc2";

typedef enum {
    BC2_PIN_MODE_UNLOCK = 0,
    BC2_PIN_MODE_CREATE,
    BC2_PIN_MODE_CONFIRM,
    BC2_PIN_MODE_AUTHORIZE_RECEIVE,
    BC2_PIN_MODE_AUTHORIZE_TRANSACTION
} bc2_pin_mode_t;

typedef struct {
    bc2_pin_security_t security;
    bc2_pin_mode_t mode;
    char first_pin[BC2_SECURITY_PIN_LENGTH + 1U];
    char receive_address[BC2_DEVICE_RECEIVE_ADDRESS_MAX];
    bc2_device_transaction_review_t transaction;
    bool active;
} bc2_pin_session_t;

static void clear_pin_text(char *pin) {
    volatile char *cursor = pin;
    size_t remaining = BC2_SECURITY_PIN_LENGTH + 1U;
    while (remaining-- > 0U) *cursor++ = '\0';
}

static void render_pin(const bc2_hal_t *hal, const bc2_pin_entry_t *entry,
                       bc2_pin_mode_t mode) {
    const char *title = mode == BC2_PIN_MODE_CREATE ? "PIN ANLEGEN" :
                        mode == BC2_PIN_MODE_CONFIRM ? "PIN WIEDERHOLEN" :
                        mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE ? "EMPFANG FREIGEBEN" :
                        mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION ? "ZAHLUNG FREIGEBEN" :
                        "PIN EINGEBEN";
    (void)bc2_device_ui_render_pin_prompt(hal, title, entry->selected_key,
                                         (unsigned int)entry->digit_count);
}

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
                           bc2_device_machine *machine,
                           bc2_device_service_t *service,
                           bc2_navigation_t *navigation,
                           bc2_pin_entry_t *pin_entry,
                           bc2_pin_session_t *pin_session) {
    bc2_button_event_t raw_event;
    bc2_button_event_t button_event;
    bc2_device_event device_event;
    const bc2_hal_result_t result = bc2_button_adapter_poll(&raw_event);

    if (result == BC2_HAL_ERROR_UNAVAILABLE || result == BC2_HAL_ERROR_NOT_FOUND) return;
    if (result != BC2_HAL_OK) {
        ESP_LOGW(TAG, "Button polling failed: %d", (int)result);
        return;
    }

    if (!bc2_navigation_process(navigation, &raw_event, &button_event)) return;

    if (pin_session->active) {
        if (button_event.button == BC2_BUTTON_RIGHT) {
            bc2_pin_entry_move_next(pin_entry);
        } else if (button_event.button == BC2_BUTTON_LEFT) {
            bc2_pin_entry_move_previous(pin_entry);
        } else if (button_event.button == BC2_BUTTON_CONFIRM) {
            const bc2_pin_entry_result_t pin_result = bc2_pin_entry_confirm(pin_entry);
            if (pin_result == BC2_PIN_ENTRY_COMPLETE) {
                bc2_device_event unlock_result = BC2_DEVICE_EVENT_UNLOCK_FAILURE;
                const uint64_t now_ms = bc2_hal_now_ms(hal);
                if (pin_session->mode == BC2_PIN_MODE_CREATE) {
                    (void)snprintf(pin_session->first_pin, sizeof(pin_session->first_pin),
                                   "%s", pin_entry->digits);
                    pin_session->mode = BC2_PIN_MODE_CONFIRM;
                    bc2_pin_entry_clear(pin_entry);
                    bc2_pin_entry_init(pin_entry);
                    render_pin(hal, pin_entry, pin_session->mode);
                    return;
                }
                if (pin_session->mode == BC2_PIN_MODE_CONFIRM) {
                    if (bc2_pin_entry_matches(pin_entry, pin_session->first_pin) &&
                        bc2_pin_security_create(&pin_session->security, hal,
                                                pin_entry->digits) == BC2_PIN_SECURITY_OK)
                        unlock_result = BC2_DEVICE_EVENT_UNLOCK_SUCCESS;
                    else
                        pin_session->mode = BC2_PIN_MODE_CREATE;
                    clear_pin_text(pin_session->first_pin);
                } else if (pin_session->mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE ||
                           pin_session->mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION) {
                    const bc2_pin_security_result_t verified = bc2_pin_security_verify(
                        &pin_session->security, hal, pin_entry->digits, now_ms);
                    bc2_pin_entry_clear(pin_entry);
                    if (verified == BC2_PIN_SECURITY_OK) {
                        char transaction_text[256];
                        const bool is_transaction =
                            pin_session->mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION;
                        if (is_transaction) {
                            (void)snprintf(transaction_text, sizeof(transaction_text),
                                           "AN: %s\nBETRAG: %llu sat\nGEBUEHR: %llu sat\nWECHSEL: %llu sat",
                                           pin_session->transaction.recipient_address,
                                           (unsigned long long)pin_session->transaction.recipient_amount,
                                           (unsigned long long)pin_session->transaction.fee_amount,
                                           (unsigned long long)pin_session->transaction.change_amount);
                        }
                        bc2_device_view_data_t view = {
                            is_transaction ? transaction_text : pin_session->receive_address,
                            "BEIDE: OK  BOOT: ABBRUCH"};
                        pin_session->active = false;
                        (void)bc2_device_machine_dispatch(machine,
                                                          is_transaction
                                                              ? BC2_DEVICE_EVENT_OPEN_TRANSACTION
                                                              : BC2_DEVICE_EVENT_OPEN_RECEIVE,
                                                          now_ms);
                        (void)bc2_device_flow_render(hal, machine, &view);
                    } else {
                        bc2_pin_entry_init(pin_entry);
                        render_pin(hal, pin_entry, pin_session->mode);
                    }
                    return;
                } else {
                    const bc2_pin_security_result_t verified = bc2_pin_security_verify(
                        &pin_session->security, hal, pin_entry->digits, now_ms);
                    if (verified == BC2_PIN_SECURITY_OK)
                        unlock_result = BC2_DEVICE_EVENT_UNLOCK_SUCCESS;
                    else if (verified == BC2_PIN_SECURITY_DELAYED)
                        ESP_LOGW(TAG, "PIN delayed for %llu ms",
                                 (unsigned long long)bc2_pin_security_remaining_delay(
                                     &pin_session->security, now_ms));
                }
                bc2_pin_entry_clear(pin_entry);
                (void)bc2_device_machine_dispatch(machine, unlock_result, now_ms);
                if (machine->state == BC2_DEVICE_UNLOCKING) {
                    bc2_pin_entry_init(pin_entry);
                    render_pin(hal, pin_entry, pin_session->mode);
                } else {
                    pin_session->active = false;
                    render_current_state(hal, machine);
                }
                return;
            }
        }
        render_pin(hal, pin_entry, pin_session->mode);
        return;
    }

    if (bc2_device_flow_event_from_button(machine->state,
                                          &button_event,
                                          &device_event) == 0)
        return;
    if (bc2_device_machine_dispatch(machine, device_event, bc2_hal_now_ms(hal)) != 0) {
        if (machine->state == BC2_DEVICE_UNLOCKING) {
            bc2_pin_entry_init(pin_entry);
            pin_session->active = true;
            render_pin(hal, pin_entry, pin_session->mode);
        } else {
            if (machine->last_action == BC2_DEVICE_ACTION_RECEIVE_CONFIRMED ||
                machine->last_action == BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED ||
                machine->last_action == BC2_DEVICE_ACTION_CANCELLED)
                memset(pin_session->receive_address, 0,
                       sizeof(pin_session->receive_address));
            if (machine->last_action == BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED ||
                machine->last_action == BC2_DEVICE_ACTION_CANCELLED) {
                if (machine->last_action == BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED)
                    bc2_device_service_complete_transaction(service, 1);
                else if (machine->state == BC2_DEVICE_DASHBOARD)
                    bc2_device_service_complete_transaction(service, 0);
                memset(&pin_session->transaction, 0, sizeof(pin_session->transaction));
            }
            render_current_state(hal, machine);
        }
    }
}

static void process_transaction_request(bc2_device_service_t *service,
                                        const bc2_hal_t *hal,
                                        const bc2_device_machine *machine,
                                        bc2_pin_entry_t *pin_entry,
                                        bc2_pin_session_t *pin_session) {
    if (machine->state != BC2_DEVICE_DASHBOARD || pin_session->active) return;
    if (!bc2_device_service_take_transaction(service, &pin_session->transaction)) return;
    pin_session->mode = BC2_PIN_MODE_AUTHORIZE_TRANSACTION;
    pin_session->active = true;
    bc2_pin_entry_clear(pin_entry);
    bc2_pin_entry_init(pin_entry);
    render_pin(hal, pin_entry, pin_session->mode);
}

static void process_receive_request(bc2_device_service_t *service,
                                    const bc2_hal_t *hal,
                                    const bc2_device_machine *machine,
                                    bc2_pin_entry_t *pin_entry,
                                    bc2_pin_session_t *pin_session) {
    if (machine->state != BC2_DEVICE_DASHBOARD || pin_session->active) return;
    if (!bc2_device_service_take_receive_address(service,
                                                  pin_session->receive_address,
                                                  sizeof(pin_session->receive_address))) return;
    pin_session->mode = BC2_PIN_MODE_AUTHORIZE_RECEIVE;
    pin_session->active = true;
    bc2_pin_entry_clear(pin_entry);
    bc2_pin_entry_init(pin_entry);
    render_pin(hal, pin_entry, pin_session->mode);
}

static void process_usb(bc2_device_service_t *service,
                        const bc2_hal_t *hal,
                        const bc2_device_machine *machine) {
    uint8_t capabilities = BC2_DEVICE_CAP_USB |
                           BC2_DEVICE_CAP_STORAGE |
                           BC2_DEVICE_CAP_RANDOM;
    if (bc2_waveshare_display_ready()) capabilities |= BC2_DEVICE_CAP_DISPLAY;
    if (bc2_waveshare_buttons_ready()) capabilities |= BC2_DEVICE_CAP_BUTTONS;

    const bc2_device_identity_t identity = {
        BC2_BOARD_NAME,
        BC2_BOARD_DISPLAY_WIDTH,
        BC2_BOARD_DISPLAY_HEIGHT,
        (uint8_t)bc2_waveshare_board_revision(),
        capabilities,
    };
    const bc2_hal_result_t result = bc2_device_service_process_usb(service,
                                                                   hal,
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

static void bc2_application_task(void *argument) {
    (void)argument;
    bc2_hal_t hal = {0};
    bc2_device_machine machine;
    bc2_device_service_t device_service;
    bc2_navigation_t navigation;
    bc2_pin_entry_t pin_entry;
    bc2_pin_session_t pin_session = {0};

    ESP_ERROR_CHECK(bc2_waveshare_bsp_init(&hal));
    ESP_ERROR_CHECK(bc2_button_adapter_init());
    if (!bc2_hal_is_complete(&hal)) {
        ESP_LOGE(TAG, "HAL incomplete");
        esp_restart();
    }

    bc2_device_machine_init(&machine, 1, bc2_hal_now_ms(&hal));
    bc2_device_service_init(&device_service);
    bc2_navigation_init(&navigation);
    bc2_pin_entry_init(&pin_entry);
    const bc2_pin_security_result_t pin_load = bc2_pin_security_load(
        &pin_session.security, &hal, bc2_hal_now_ms(&hal));
    if (pin_load == BC2_PIN_SECURITY_NOT_CONFIGURED)
        pin_session.mode = BC2_PIN_MODE_CREATE;
    else if (pin_load != BC2_PIN_SECURITY_OK) {
        ESP_LOGE(TAG, "PIN storage invalid");
        (void)bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                                          bc2_hal_now_ms(&hal));
    }
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
        /* Physical input has priority and must never wait behind USB I/O. */
        process_button(&hal, &machine, &device_service, &navigation, &pin_entry, &pin_session);
        process_usb(&device_service, &hal, &machine);
        process_receive_request(&device_service, &hal, &machine, &pin_entry, &pin_session);
        process_transaction_request(&device_service, &hal, &machine, &pin_entry, &pin_session);
        if (bc2_device_machine_tick(&machine, bc2_hal_now_ms(&hal)) != 0) {
            if (machine.state != BC2_DEVICE_TRANSACTION_REVIEW)
                bc2_device_service_complete_transaction(&device_service, 0);
            render_current_state(&hal, &machine);
        }
        vTaskDelay(pdMS_TO_TICKS(BC2_DEVICE_LOOP_DELAY_MS));
    }
}

void app_main(void) {
    const BaseType_t created = xTaskCreate(bc2_application_task,
                                           "bc2_application",
                                           BC2_APPLICATION_TASK_STACK_SIZE,
                                           NULL,
                                           BC2_APPLICATION_TASK_PRIORITY,
                                           NULL);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Unable to create BC2 application task");
        esp_restart();
    }
}
