# Wallet Recovery

## User workflow

Recovery is used to restore a BC2 wallet from its recovery words.

1. Connect the BC2 hardware wallet by USB.
2. Choose **Recovery Wallet** when the device state permits recovery.
3. Select/enter the supported 12- or 24-word BIP39 recovery phrase in the desktop recovery dialog.
4. The desktop validates the phrase and transfers it to the hardware for the recovery operation.
5. Follow the hardware instructions.

Depending on the device state:

- On a device with an existing wallet outside LOCKDOWN, the existing device PIN may be required before replacement/recovery completes.
- In LOCKDOWN, the workflow requires creation and confirmation of a new four-digit PIN.
- On a fresh device, recovery is expected to install the recovered wallet as the single device wallet.

## Security handling

The recovery phrase is highly sensitive.

- Never share it with another person.
- Never enter it into websites or unrelated applications.
- The BC2 desktop must not persist or log the mnemonic.
- Recovery input is transferred to the connected BC2 hardware only for the recovery workflow.

## Single-wallet behavior

A BC2 device manages one wallet at a time. Recovering another wallet replaces the wallet currently associated with the device according to the product workflow. Public desktop cache data is separated by wallet identity and is not wallet authority.

## Test status

Recovery functionality has been used successfully during development, including wallet replacement/recovery workflows. However, the specific release-gate cycle `third wrong PIN -> LOCKDOWN -> Recovery -> new PIN -> DASHBOARD` was not completed in the 2026-08-24 protocol-verification session and remains explicitly pending in `RELEASE_CHECKLIST.md`.
