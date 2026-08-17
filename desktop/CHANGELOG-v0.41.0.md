# v0.42.1

- Fixed BC2 USB response parsing on ESP32-S3 USB Serial/JTAG.
- ESP-IDF console logs and BC2 protocol frames share the same serial stream; log text can contain the literal string `BC2`.
- The old desktop parser treated such log text as a binary frame and could report `payload limit exceeded` or `no BC2 response received`.
- The parser now validates protocol version, expected response command, request sequence and payload length before accepting a frame.
- This fix applies to Receive, Wallet sperren/Logout and all other Desktop-to-Hardware commands.
