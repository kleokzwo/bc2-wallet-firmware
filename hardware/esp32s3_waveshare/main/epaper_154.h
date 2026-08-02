#ifndef BC2_EPAPER_154_H
#define BC2_EPAPER_154_H

#include "esp_err.h"
#include <stdbool.h>

#define BC2_EPAPER_WIDTH 200
#define BC2_EPAPER_HEIGHT 200

esp_err_t bc2_epaper_init(void);
esp_err_t bc2_epaper_show_text(const char *title, const char *body, const char *footer,
                               bool full_refresh);
bool bc2_epaper_is_ready(void);

#endif
