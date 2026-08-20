# BC2 Cold Wallet Firmware v0.45.0

## Phase 1 – Wallet-ID Contract

- Added USB command `GET_WALLET_ID` (`0x48`).
- Wallet ID is available only while the authenticated device is in `DASHBOARD`.
- Added deterministic 128-bit wallet ID derived entirely on hardware from the BIP84 account public key.
- Derivation is domain-separated with `BC2 wallet id v1` and SHA-256; only the first 16 digest bytes are exposed.
- Seed, mnemonic, private keys and PIN never form part of the USB response.
- Wallet ID is cleared from the USB service context when the device locks.
- Same recovered wallet produces the same wallet ID; another wallet produces another ID.

This release intentionally does not add desktop cache behavior yet. That belongs to the next phase.
