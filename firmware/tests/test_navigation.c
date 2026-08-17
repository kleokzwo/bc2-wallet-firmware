#include "bc2_navigation.h"

#include <assert.h>

int main(void) {
    bc2_navigation_t navigation;
    bc2_button_event_t output = {0};
    bc2_button_event_t input = {BC2_BUTTON_RIGHT, BC2_BUTTON_PRESSED, 100U};

    bc2_navigation_init(&navigation);
    assert(!bc2_navigation_process(&navigation, &input, &output));
    input.action = BC2_BUTTON_RELEASED;
    input.timestamp_ms = 250U;
    assert(bc2_navigation_process(&navigation, &input, &output));
    assert(output.button == BC2_BUTTON_RIGHT);
    assert(output.action == BC2_BUTTON_PRESSED);

    input.button = BC2_BUTTON_LEFT;
    input.action = BC2_BUTTON_PRESSED;
    input.timestamp_ms = 1000U;
    assert(!bc2_navigation_process(&navigation, &input, &output));
    input.button = BC2_BUTTON_RIGHT;
    input.timestamp_ms = 1050U;
    assert(bc2_navigation_process(&navigation, &input, &output));
    assert(output.button == BC2_BUTTON_CONFIRM);
    input.button = BC2_BUTTON_LEFT;
    input.action = BC2_BUTTON_RELEASED;
    assert(!bc2_navigation_process(&navigation, &input, &output));
    input.button = BC2_BUTTON_RIGHT;
    assert(!bc2_navigation_process(&navigation, &input, &output));
    return 0;
}
