#include "bc2_device_ui.h"
#include "bc2_hal.h"
#include "waveshare_bsp.h"
#include "esp_log.h"
#include "esp_system.h"
#include <stdio.h>
void app_main(void) {
    bc2_hal_t hal={0};
    ESP_ERROR_CHECK(bc2_waveshare_bsp_init(&hal));
    if(!bc2_hal_is_complete(&hal)){ESP_LOGE("bc2","HAL incomplete");esp_restart();}
    char info[160];
    (void)snprintf(info,sizeof(info),"Firmware 0.9.0\n%s\nDisplay %ux%u\nSAFE HARDWARE BRING-UP",
                   BC2_BOARD_NAME,BC2_BOARD_DISPLAY_WIDTH,BC2_BOARD_DISPLAY_HEIGHT);
    (void)bc2_device_ui_render(&hal,BC2_DEVICE_SCREEN_BOOT,info,NULL);
    uint8_t rng_probe[32];
    if(bc2_hal_random(&hal,rng_probe,sizeof(rng_probe))!=BC2_HAL_OK) ESP_LOGE("bc2","RNG self-test failed");
    ESP_LOGI("bc2","v0.9.0 hardware reference started; Wi-Fi/BLE are not initialized");
}
