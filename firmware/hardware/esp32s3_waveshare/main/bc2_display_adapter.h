#ifndef BC2_DISPLAY_ADAPTER_H
#define BC2_DISPLAY_ADAPTER_H
#include "esp_err.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t bc2_display_adapter_init(void);
esp_err_t bc2_display_adapter_show_text(const char *title, const char *body, const char *footer, bool full_refresh);
#ifdef __cplusplus
}
#endif
#endif
