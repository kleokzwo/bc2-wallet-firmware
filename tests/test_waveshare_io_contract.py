#!/usr/bin/env python3
"""Regression guard: hardware buttons must not be starved by USB polling."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BSP = (ROOT / "hardware/esp32s3_waveshare/main/waveshare_bsp.c").read_text()
APP = (ROOT / "hardware/esp32s3_waveshare/main/app_main.c").read_text()

assert "#define BC2_USB_RX_TIMEOUT_TICKS 0U" in BSP
assert "BC2_USB_IO_TIMEOUT_TICKS" not in BSP
assert APP.index("process_button(&hal") < APP.index("process_usb(&device_service")

print("Waveshare I/O contract: OK")
