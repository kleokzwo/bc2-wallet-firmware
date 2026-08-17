#ifndef BC2_DEVICE_FLOW_H
#define BC2_DEVICE_FLOW_H

#include "bc2_device_state.h"
#include "bc2_device_ui.h"
#include "bc2_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *primary_text;
    const char *secondary_text;
} bc2_device_view_data_t;

bc2_device_screen_t bc2_device_flow_screen_for_state(bc2_device_state state);
int bc2_device_flow_event_from_button(bc2_device_state state,
                                      const bc2_button_event_t *button_event,
                                      bc2_device_event *device_event);
bc2_hal_result_t bc2_device_flow_render(const bc2_hal_t *hal,
                                        const bc2_device_machine *machine,
                                        const bc2_device_view_data_t *view_data);

#ifdef __cplusplus
}
#endif

#endif
