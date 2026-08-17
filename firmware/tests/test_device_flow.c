#include "bc2_device_flow.h"

#include <assert.h>
#include <string.h>

typedef struct {
    bc2_display_frame_t frame;
} fake_display_t;

static bc2_hal_result_t present(void *context, const bc2_display_frame_t *frame) {
    fake_display_t *display = (fake_display_t *)context;
    display->frame = *frame;
    return BC2_HAL_OK;
}

int main(void) {
    bc2_device_machine machine;
    bc2_device_event device_event = BC2_DEVICE_EVENT_FATAL_ERROR;
    bc2_button_event_t button = {BC2_BUTTON_CONFIRM, BC2_BUTTON_PRESSED, 100U};
    fake_display_t display = {0};
    bc2_hal_t hal = {0};
    bc2_device_view_data_t view = {"bc21qexample", "Fee 100 sat"};

    hal.context = &display;
    hal.display_present = present;

    bc2_device_machine_init(&machine, 1, 0U);
    assert(bc2_device_flow_screen_for_state(machine.state) == BC2_DEVICE_SCREEN_BOOT);

    assert(bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    assert(machine.state == BC2_DEVICE_LOCKED);
    assert(bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    assert(device_event == BC2_DEVICE_EVENT_BEGIN_UNLOCK);

    assert(bc2_device_machine_dispatch(&machine, device_event, 2U));
    assert(bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_UNLOCK_SUCCESS, 3U));
    assert(machine.state == BC2_DEVICE_DASHBOARD);

    button.action = BC2_BUTTON_LONG_PRESSED;
    assert(bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    assert(device_event == BC2_DEVICE_EVENT_LOCK);
    button.action = BC2_BUTTON_PRESSED;

    button.button = BC2_BUTTON_LEFT;
    assert(!bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    assert(bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_OPEN_RECEIVE, 4U));

    assert(bc2_device_flow_render(&hal, &machine, &view) == BC2_HAL_OK);
    assert(strcmp(display.frame.title, "ADRESSE PRUEFEN") == 0);
    assert(strcmp(display.frame.body, "bc21qexample") == 0);

    button.button = BC2_BUTTON_BACK;
    assert(bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    assert(device_event == BC2_DEVICE_EVENT_CANCEL);

    button.button = BC2_BUTTON_LEFT;
    assert(bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    assert(device_event == BC2_DEVICE_EVENT_CANCEL);

    button.action = BC2_BUTTON_RELEASED;
    assert(!bc2_device_flow_event_from_button(machine.state, &button, &device_event));
    return 0;
}
