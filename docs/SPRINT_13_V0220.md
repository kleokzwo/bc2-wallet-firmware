# Sprint 13 - v0.22.0 Hardware Display Foundation

## Binding decision

The BC2 project no longer maintains a custom E-paper controller driver. The manufacturer-provided Waveshare V2 BSP from the exact demo verified on the physical board is included as a dedicated hardware component.

## Architecture

- C17 wallet/device core remains independent from ESP-IDF and Qt.
- `waveshare_bsp.c` implements the BC2 HAL.
- `bc2_display_adapter.cpp` translates BC2 display frames into pixels.
- `epaper_driver_bsp` and `board_power_bsp` remain manufacturer hardware code.

## Acceptance test

1. Build with ESP-IDF 5.5.3.
2. Flash to ESP32-S3-PICO-1 N8R8.
3. The old Waveshare demo image must first clear.
4. A BC2 text screen must appear.
5. USB probe must continue to report firmware 0.22.0 and DISPLAY/BUTTONS capabilities.
