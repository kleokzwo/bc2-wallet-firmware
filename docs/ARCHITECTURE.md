# Architecture

## Purpose

BC2 Cold Wallet is a BC2-only hardware-wallet system composed of an ESP32-S3 hardware device and a PySide6 desktop application. The design keeps security-critical wallet operations on the hardware while the desktop provides network access and user interface functions.

## High-level design

```text
BC2 network / Electrum
        |
        v
Desktop application
  - UI and navigation
  - Electrum/network access
  - public wallet cache
  - transaction preparation
        |
        | framed BC2 protocol over USB
        v
Hardware wallet (ESP32-S3)
  - device state machine
  - PIN enforcement
  - wallet initialization
  - seed/private-key custody
  - address confirmation
  - transaction review/signing
  - recovery acceptance
```

## Single-wallet device policy

A BC2 hardware device manages exactly one wallet at a time.

- Factory state: no wallet is installed.
- Create or Recovery installs one wallet.
- Installing/recovering another wallet replaces the currently installed wallet according to the product workflow.
- Normal operation does not expose multiple simultaneously active hardware wallets.

The desktop keeps public cache data isolated by hardware wallet ID. The cache is non-authoritative and has no API for seeds, mnemonic phrases, PINs, private keys, or signing secrets.

## Wallet creation

For a new wallet, the intended security boundary is:

1. Desktop requests wallet creation.
2. The user establishes the required 4-digit PIN on the hardware.
3. The hardware creates the wallet.
4. The 12 recovery words are displayed on the hardware device.
5. Recovery words are not intended to be generated or displayed by the desktop.

## Recovery

Recovery is desktop-assisted:

1. The user chooses Recovery Wallet on the desktop.
2. The desktop validates a 12- or 24-word BIP39 mnemonic.
3. The mnemonic is transferred to the connected hardware for the recovery operation.
4. The desktop recovery dialog clears its in-memory reference after submission and the application must not persist or log the mnemonic.
5. Hardware state and PIN policy determine the remaining flow.

In LOCKDOWN, recovery requires creation of a new 4-digit PIN before returning to the unlocked wallet state.

## Desktop responsibilities

The desktop application is responsible for presentation, device discovery, public wallet data, network synchronization, transaction preparation, and orchestration of hardware requests. Security-critical confirmation remains a hardware responsibility.

Current navigation is modularized into dedicated pages for Dashboard, Receive, Send, Transactions, Device, Settings, and About.

## Hardware responsibilities

The hardware is the authority for device state and wallet security. It implements the state machine, PIN attempts, automatic session locking, recovery state, wallet initialization status, display/button confirmation, and signing-related device operations.

## Transport boundary

The native USB Serial/JTAG endpoint carries the framed BC2 binary protocol. ESP-IDF diagnostic console output must not be mixed into that byte stream. Runtime diagnostic logging is kept separate on UART0.

A single process should own the BC2 serial endpoint while exchanging frames. Running `idf.py monitor`, the desktop application, and a protocol probe concurrently against the same `/dev/ttyACM0` can split the byte stream between readers and produce misleading partial frames.
