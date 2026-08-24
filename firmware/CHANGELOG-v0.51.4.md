# BC2 Cold Wallet v0.51.4 - Clean USB Boot + Human-readable Probe

## USB protocol isolation
- ESP-IDF runtime console stays on UART0.
- Secondary USB Serial/JTAG console remains disabled.
- Bootloader logging is disabled (`CONFIG_BOOTLOADER_LOG_LEVEL_NONE=y`).
- ROM boot logging is disabled (`CONFIG_BOOT_ROM_LOG_ALWAYS_OFF=y`).
- Stale `build/` trees and `sdkconfig.old` are intentionally excluded from this source archive.

Goal: the native USB Serial/JTAG endpoint carries framed BC2 protocol bytes only; no boot or ESP-IDF text may precede a BC2 frame.

## Probe
- Added root-level `bc2_probe.py` for easy execution.
- Human-readable field decoding for magic, version, command, response bit, sequence, payload length, device state and wallet status.
- Explicit detection and ASCII display of any foreign bytes on the BC2 USB channel.
- Startup traffic is captured separately so boot text cannot be mistaken for a BC2 response.
- Probe sends read-only/status requests only; no PIN, seed, mnemonic, private key or signing request is transmitted.
