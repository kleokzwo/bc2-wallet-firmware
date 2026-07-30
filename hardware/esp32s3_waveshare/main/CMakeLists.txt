idf_component_register(
    SRCS "app_main.c" "waveshare_bsp.c"
         "../../../firmware/platform/bc2_hal.c"
         "../../../firmware/platform/bc2_device_ui.c"
         "../../../firmware/protocol/bc2_usb_protocol.c"
    INCLUDE_DIRS "." "../../../firmware/include"
    REQUIRES esp_timer esp_system nvs_flash driver
)
