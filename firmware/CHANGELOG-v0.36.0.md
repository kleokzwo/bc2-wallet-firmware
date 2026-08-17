# BC2 Firmware v0.36.0

- Onboarding no longer forces PIN creation immediately after boot on an empty device.
- Create New Wallet now starts the 4-digit PIN setup only after the user selected wallet creation.
- Added USB commands for Unlock Wallet and Recovery Wallet.
- Recovery can be selected while an existing wallet is still locked; replacing it requires physical confirmation on the hardware.
- Recovery words are entered only on the hardware. 12-word and 24-word BIP39 recovery are supported.
- Added encrypted 24-word entropy storage while retaining compatibility with the existing 12-word wallet record.
- Receive requests remain hardware-owned and are accepted only in the unlocked Dashboard state.
- PIN length remains exactly four digits.
