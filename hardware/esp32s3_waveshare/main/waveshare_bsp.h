#ifndef WAVESHARE_BSP_H
#define WAVESHARE_BSP_H
#include "bc2_hal.h"
#include "esp_err.h"
#define BC2_BOARD_NAME "Waveshare ESP32-S3-ePaper-1.54"
#define BC2_BOARD_DISPLAY_WIDTH 200U
#define BC2_BOARD_DISPLAY_HEIGHT 200U
typedef enum { BC2_BOARD_REVISION_UNKNOWN=0, BC2_BOARD_REVISION_V1, BC2_BOARD_REVISION_V2 } bc2_board_revision_t;
esp_err_t bc2_waveshare_bsp_init(bc2_hal_t *hal);
bc2_board_revision_t bc2_waveshare_board_revision(void);
#endif
