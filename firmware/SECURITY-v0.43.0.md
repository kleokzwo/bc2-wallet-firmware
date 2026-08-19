# BC2 v0.43.0 Radio-Off Development Profile

- Bluetooth/BLE disabled at build level.
- Application source contains no intentional Wi-Fi/Bluetooth initialization.
- Compile-time guard rejects a Bluetooth-enabled build.
- Boot log states radio policy.
- No eFuses changed.
- Secure Boot V2 / Flash Encryption intentionally deferred to production provisioning.
- Seed, PIN, signing and USB wallet protocol unchanged.

Application radio references found before change: 0
None
