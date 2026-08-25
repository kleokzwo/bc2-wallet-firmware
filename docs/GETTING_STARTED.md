# Getting Started

## Connect the device

Connect the BC2 Cold Wallet to the computer using USB and start the BC2 desktop application.

The available start actions depend on the state reported by the hardware.

## New / empty device

A fresh device is designed to offer:

- **Create New Wallet** — create a new BC2 wallet on the hardware.
- **Recovery Wallet** — restore an existing wallet from recovery words.

During new-wallet creation, follow the instructions on the hardware. The device PIN is exactly four digits. Recovery words for a newly created wallet are intended to be generated and displayed on the hardware device.

> Release note: the fresh-device UI/state path is source-defined but remains a pending physical release-gate test as documented in `SECURITY_TESTS.md`.

## Device with an installed wallet

A locked device with an installed wallet offers **Unlock Wallet**. Enter the four-digit PIN on the hardware. After successful unlock the desktop opens the wallet dashboard.

## Security LOCKDOWN

After three incorrect PIN attempts, the hardware enters LOCKDOWN. The normal Unlock action is no longer available; use **Recovery Wallet** and follow the recovery instructions.

## Basic safety

- Keep recovery words offline and private.
- Never share recovery words or the PIN.
- Verify receive addresses on the hardware display.
- Review transaction details on the hardware before confirming/signing.
- Treat the desktop/network as supporting components; the hardware display is the security confirmation point for critical actions.
