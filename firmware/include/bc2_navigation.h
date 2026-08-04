#ifndef BC2_NAVIGATION_H
#define BC2_NAVIGATION_H

#include "bc2_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_NAVIGATION_LONG_PRESS_MS 800U

typedef struct {
    int left_pressed;
    int right_pressed;
    int chord_active;
} bc2_navigation_t;

void bc2_navigation_init(bc2_navigation_t *navigation);
int bc2_navigation_process(bc2_navigation_t *navigation,
                           const bc2_button_event_t *raw_event,
                           bc2_button_event_t *navigation_event);

#ifdef __cplusplus
}
#endif

#endif
