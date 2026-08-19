# BC2 Hardware v0.42.1

- ESP32-S3 target clock: 240 MHz
- Main device loop: 50 ms -> 10 ms
- Screen changes no longer force a full E-Paper refresh
- Full refresh still occurs when explicitly requested or after 12 partial refreshes
- Larger/adaptive text on PIN, Receive and short body messages
- Locked screen simplified to the supplied BC2-II logo only
- No seed/PIN/signing/USB security protocol logic changed
