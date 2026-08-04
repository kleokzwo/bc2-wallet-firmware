idf_component_register(
    SRCS "app_main.c" "waveshare_bsp.c" "bc2_display_adapter.cpp" "bc2_button_adapter.c"
         "../../../firmware/platform/bc2_hal.c"
         "../../../firmware/platform/bc2_device_ui.c"
         "../../../firmware/platform/bc2_device_state.c"
         "../../../firmware/platform/bc2_navigation.c"
         "../../../firmware/platform/bc2_pin_entry.c"
         "../../../firmware/platform/bc2_pin_security.c"
         "../../../firmware/platform/bc2_device_flow.c"
         "../../../firmware/platform/bc2_device_service.c"
         "../../../firmware/platform/bc2_receive_request.c"
         "../../../firmware/protocol/bc2_usb_protocol.c"
         "../../../firmware/protocol/bc2_usb_stream.c"
    INCLUDE_DIRS "." "../../../firmware/include"
    REQUIRES esp_timer esp_system nvs_flash driver esp_driver_usb_serial_jtag mbedtls epaper_driver_bsp board_power_bsp
)
