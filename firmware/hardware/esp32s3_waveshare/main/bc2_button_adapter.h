#ifndef BC2_BUTTON_ADAPTER_H
#define BC2_BUTTON_ADAPTER_H

#include "bc2_hal.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bc2_button_adapter_init(void);
bc2_hal_result_t bc2_button_adapter_poll(bc2_button_event_t *event);

#ifdef __cplusplus
}
#endif

#endif
