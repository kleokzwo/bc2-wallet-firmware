# BC2 Cold Wallet Desktop v0.36.0

- Desktop onboarding now follows the actual hardware state instead of wallet_status alone.
- Empty device: Create New Wallet and Recovery Wallet are shown before any PIN is requested.
- Existing locked wallet: Unlock Wallet and Recovery Wallet are shown; Dashboard is not opened until the hardware is unlocked.
- Recovery is started on the hardware and never requests mnemonic words in the desktop application.
- Receive is blocked while the device is locked, preventing the previous desktop/firmware state mismatch.
- Updated desktop/firmware USB protocol for recovery and unlock requests.
