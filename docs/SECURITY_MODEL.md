# Security Model

## Security objectives

The BC2 Cold Wallet design aims to keep wallet secrets and authorization decisions on the hardware device while treating the desktop and network as less trusted components.

Core rules:

- Private keys must not leave the hardware wallet.
- Newly created recovery words are generated/displayed on the hardware, not the desktop.
- Recovery is desktop-assisted, but the recovery mnemonic must not be persisted or logged by the desktop.
- Security-critical actions require the device's 4-digit PIN and/or explicit hardware confirmation as appropriate.
- Receive addresses are reviewed on the hardware.
- Transactions are reviewed and confirmed on the hardware before signing.
- One device manages one installed wallet at a time.

## PIN policy

The device PIN is exactly four numeric digits.

The firmware state machine allows a maximum of three failed unlock attempts. Internal hardware tests on 2026-08-24 confirmed:

- failed attempt 1 -> device remains LOCKED;
- failed attempt 2 -> device remains LOCKED;
- failed attempt 3 -> device enters LOCKDOWN.

LOCKDOWN requires the recovery workflow rather than another normal unlock attempt.

## Locking

The firmware has an automatic unlocked-session timeout of five minutes. The 2026-08-24 hardware test confirmed that an unlocked DASHBOARD state returned to LOCKED automatically while the wallet remained initialized.

The desktop also exposes a `Wallet sperren` action which issues the hardware lock command. The automatic hardware lock was directly verified during the recorded session; see `SECURITY_TESTS.md` for exact test status.

## Lockdown and recovery

LOCKDOWN is a security state, not a wallet-deletion state. The recorded test confirmed that after the third wrong PIN:

```text
Device state  = LOCKDOWN (0x0A)
Wallet status = INITIALIZED (0x02)
```

The complete `LOCKDOWN -> Recovery -> new PIN -> DASHBOARD` cycle was not completed in the recorded session and therefore remains a release-gate item.

## Factory state

Source-defined expected factory state:

```text
Device state  = SETUP_REQUIRED (0x01)
Wallet status = NOT_INITIALIZED (0x00)
```

The desktop source maps a device without an initialized wallet to a setup screen where `Create New Wallet` and `Recovery Wallet` are visible and normal `Unlock Wallet` is hidden.

This fresh-device state was not physically verified on 2026-08-24 because only one test device was available and it contained the active test wallet. It must not be represented as a passed hardware test until executed.

## Desktop public-data cache

Public UX/performance data is isolated by wallet ID. Cached data may include receive addresses, balances, transactions, and last-sync information. It is non-authoritative. Seeds, mnemonic phrases, PINs, private keys, and signing secrets are outside the cache API.

## USB protocol isolation

A security/stability defect was found where ESP-IDF text logging shared the BC2 protocol endpoint. The v0.51.3-v0.51.5 hardening series separated diagnostic logging from the binary BC2 channel and hardened complete-frame transmission/diagnostics.

The resulting protocol tests confirmed clean BC2 frames without ESP-IDF log text mixed into responses.

## Security claims

The repository may describe these checks as internal security, protocol, and integration tests. It must not describe the product as independently audited, certified, or externally penetration-tested unless such work is actually completed and documented by the responsible third party.
