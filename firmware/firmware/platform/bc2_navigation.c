#include "bc2_navigation.h"

#include <string.h>

void bc2_navigation_init(bc2_navigation_t *navigation) {
    if (navigation == NULL) return;
    memset(navigation, 0, sizeof(*navigation));
}

int bc2_navigation_process(bc2_navigation_t *navigation,
                           const bc2_button_event_t *raw_event,
                           bc2_button_event_t *navigation_event) {
    if (navigation == NULL || raw_event == NULL || navigation_event == NULL) return 0;
    if (raw_event->button != BC2_BUTTON_LEFT && raw_event->button != BC2_BUTTON_RIGHT)
        return 0;

    if (raw_event->action == BC2_BUTTON_PRESSED) {
        if (raw_event->button == BC2_BUTTON_LEFT) navigation->left_pressed = 1;
        else navigation->right_pressed = 1;
        if (navigation->left_pressed && navigation->right_pressed && !navigation->chord_active) {
            navigation->chord_active = 1;
            *navigation_event = *raw_event;
            navigation_event->button = BC2_BUTTON_CONFIRM;
            return 1;
        }
        return 0;
    }

    if (raw_event->action != BC2_BUTTON_RELEASED) return 0;
    if (raw_event->button == BC2_BUTTON_LEFT) navigation->left_pressed = 0;
    else navigation->right_pressed = 0;

    if (navigation->chord_active) {
        if (!navigation->left_pressed && !navigation->right_pressed)
            navigation->chord_active = 0;
        return 0;
    }

    *navigation_event = *raw_event;
    navigation_event->action = BC2_BUTTON_PRESSED;
    return 1;
}
