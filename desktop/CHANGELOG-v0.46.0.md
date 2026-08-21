# BC2 Desktop v0.46.0 — Phase 2 WalletContext

- Added a deliberately small RAM-only `WalletContext`.
- `activate(wallet_id)` accepts only the 128-bit hexadecimal Wallet-ID contract from firmware v0.45.0.
- `deactivate()` removes the authenticated wallet identity from the active desktop session.
- MainWindow activates the context only after the hardware reports an unlocked wallet and `GET_WALLET_ID` succeeds.
- MainWindow deactivates the context on successful wallet lock/logout.
- No wallet cache, settings migration, transaction persistence or multi-wallet manager was added.
- If an unlocked device cannot provide a valid Wallet-ID, the desktop refuses to enter the authenticated UI.

Tests: WalletContext unit tests plus existing desktop protocol tests pass.
