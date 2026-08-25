# BC2 Cold Wallet User Guide

## Device model

BC2 Cold Wallet is designed as a single-wallet hardware device. At any moment the hardware contains either no wallet or one installed wallet.

## Unlocking

When a wallet is installed and the device is locked, the desktop presents **Unlock Wallet**. Enter the four-digit PIN on the hardware. A successful PIN changes the device into its unlocked Dashboard state.

## Receiving BC2

Use the Receive page in the desktop application. The receive workflow is designed so the address can be reviewed/confirmed on the hardware device. Always compare/confirm the address on the hardware before relying on it.

## Sending BC2

The desktop prepares the transaction and guides the user through review. Transaction details must be reviewed on the hardware before signing. The hardware is the security confirmation point for signing-related actions.

## Transactions and balances

The desktop synchronizes public blockchain information through its configured Electrum service. Public cached information can be used for UX/performance, but it is not the authority for private keys or wallet secrets.

## Locking

Use **Wallet sperren** to end the active wallet session. The hardware also has an automatic session timeout and returns to the locked state after inactivity.

## Three incorrect PINs

The third consecutive incorrect PIN causes security LOCKDOWN. Use Recovery rather than repeatedly attempting normal unlock.

## Recovery

Recovery supports 12- or 24-word BIP39 recovery phrases. Enter the phrase only in the official BC2 desktop recovery workflow while the intended BC2 hardware device is connected. The application must not persist or log the recovery phrase.

## Replacing a wallet

Because the device follows a single-wallet policy, creating/recovering another wallet replaces the currently installed wallet according to the supported workflow. Ensure the old wallet can be recovered before intentionally replacing it.

## Factory Reset

Factory Reset is destructive. Ensure recovery words are safely available before resetting a wallet that contains funds.
