# BC2 Cold Wallet v0.51.3 — USB Protocol Isolation Fix

## Fixed

- Reserved ESP32-S3 USB Serial/JTAG exclusively for the framed BC2 binary protocol.
- Disabled the ESP-IDF secondary console on USB Serial/JTAG.
- ESP-IDF / display / performance log lines are no longer mirrored into the BC2 RX/TX byte stream.
- Primary diagnostic logging remains on UART0.

## Why

`bc2_probe.py` reproduced console text (for example `bc2_display: PERF ...`) mixed into otherwise valid `42 43 32 ...` BC2 frames. The desktop transport expects a clean framed byte stream, so the shared console/protocol channel could cause intermittent parse failures, connection errors, or stalls.

## Required rebuild

Run `idf.py fullclean` before rebuilding/flashing so the generated ESP-IDF configuration cannot retain the old secondary-console setting.
