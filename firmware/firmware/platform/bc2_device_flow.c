#include "bc2_device_flow.h"

static int pressed(const bc2_button_event_t *event) {
    return event != NULL &&
           (event->action == BC2_BUTTON_PRESSED ||
            event->action == BC2_BUTTON_LONG_PRESSED);
}

bc2_device_screen_t bc2_device_flow_screen_for_state(bc2_device_state state) {
    switch (state) {
        case BC2_DEVICE_BOOT:
        case BC2_DEVICE_SETUP_REQUIRED:
            return BC2_DEVICE_SCREEN_BOOT;
        case BC2_DEVICE_LOCKED:
        case BC2_DEVICE_UNLOCKING:
        case BC2_DEVICE_COOLDOWN:
            return BC2_DEVICE_SCREEN_LOCKED;
        case BC2_DEVICE_DASHBOARD:
        case BC2_DEVICE_SETTINGS:
            return BC2_DEVICE_SCREEN_DASHBOARD;
        case BC2_DEVICE_RECEIVE_REVIEW:
            return BC2_DEVICE_SCREEN_RECEIVE_REVIEW;
        case BC2_DEVICE_TRANSACTION_REVIEW:
            return BC2_DEVICE_SCREEN_TRANSACTION_SUMMARY;
        case BC2_DEVICE_ERROR:
        default:
            return BC2_DEVICE_SCREEN_ERROR;
    }
}

int bc2_device_flow_event_from_button(bc2_device_state state,
                                      const bc2_button_event_t *button_event,
                                      bc2_device_event *device_event) {
    if (!pressed(button_event) || device_event == NULL) return 0;

    if (button_event->action == BC2_BUTTON_LONG_PRESSED &&
        button_event->button == BC2_BUTTON_CONFIRM &&
        bc2_device_machine_is_unlocked(&(bc2_device_machine){.state = state})) {
        *device_event = BC2_DEVICE_EVENT_LOCK;
        return 1;
    }

    switch (state) {
        case BC2_DEVICE_LOCKED:
            if (button_event->button == BC2_BUTTON_CONFIRM) {
                *device_event = BC2_DEVICE_EVENT_BEGIN_UNLOCK;
                return 1;
            }
            break;
        case BC2_DEVICE_UNLOCKING:
            if (button_event->button == BC2_BUTTON_BACK ||
                button_event->button == BC2_BUTTON_LEFT) {
                *device_event = BC2_DEVICE_EVENT_CANCEL;
                return 1;
            }
            break;
        case BC2_DEVICE_DASHBOARD:
            if (button_event->button == BC2_BUTTON_RIGHT) {
                *device_event = BC2_DEVICE_EVENT_OPEN_SETTINGS;
                return 1;
            }
            if (button_event->button == BC2_BUTTON_CONFIRM) {
                *device_event = BC2_DEVICE_EVENT_OPEN_TRANSACTION;
                return 1;
            }
            break;
        case BC2_DEVICE_RECEIVE_REVIEW:
        case BC2_DEVICE_TRANSACTION_REVIEW:
        case BC2_DEVICE_SETTINGS:
            if (button_event->button == BC2_BUTTON_CONFIRM) {
                *device_event = BC2_DEVICE_EVENT_CONFIRM;
                return 1;
            }
            if (button_event->button == BC2_BUTTON_BACK ||
                button_event->button == BC2_BUTTON_LEFT) {
                *device_event = BC2_DEVICE_EVENT_CANCEL;
                return 1;
            }
            break;
        case BC2_DEVICE_ERROR:
            if (button_event->button == BC2_BUTTON_BACK) {
                *device_event = BC2_DEVICE_EVENT_RECOVER;
                return 1;
            }
            break;
        default:
            break;
    }
    return 0;
}

bc2_hal_result_t bc2_device_flow_render(const bc2_hal_t *hal,
                                        const bc2_device_machine *machine,
                                        const bc2_device_view_data_t *view_data) {
    const char *primary = "";
    const char *secondary = "";
    if (machine == NULL) return BC2_HAL_ERROR_ARGUMENT;
    if (view_data != NULL) {
        if (view_data->primary_text != NULL) primary = view_data->primary_text;
        if (view_data->secondary_text != NULL) secondary = view_data->secondary_text;
    }
    return bc2_device_ui_render(hal,
                                bc2_device_flow_screen_for_state(machine->state),
                                primary,
                                secondary);
}
