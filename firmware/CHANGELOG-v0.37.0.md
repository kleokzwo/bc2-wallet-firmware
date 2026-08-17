# BC2 Firmware v0.41.0

- Replaced unusable on-device BIP39 word selection with desktop-assisted recovery transport.
- Hardware independently validates 12/24-word BIP39 recovery data.
- Hardware displays recovery fingerprint and requires physical confirmation before replacing/restoring a wallet.
- Recovery payload is held transiently and cleared after processing.
- New wallet generation remains hardware-only.
