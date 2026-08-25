# Factory Reset and Fresh Device State

## Warning

A Factory Reset is a destructive security operation. Before resetting a device that contains a wallet, ensure that the correct recovery words are safely available. Do not use a real-value wallet for destructive development tests.

## Expected fresh-device state

According to the current firmware state machine, a device without an initialized wallet boots to:

```text
Device-State : 0x01 = SETUP_REQUIRED
Wallet-Status: 0x00 = NOT_INITIALIZED
```

According to the current desktop UI logic, this state should present:

- **Create New Wallet**
- **Recovery Wallet**

and should hide the normal **Unlock Wallet** action.

## Expected Create New Wallet lifecycle

```text
SETUP_REQUIRED / NOT_INITIALIZED
        |
        +--> Create New Wallet
                |
                +--> establish 4-digit PIN on hardware
                +--> hardware creates wallet
                +--> recovery words displayed on hardware
                +--> wallet becomes INITIALIZED
                +--> normal unlocked/dashboard state
```

## Verification status

**NOT TESTED on physical fresh hardware during the 2026-08-24 release-gate session.**

Reason: only one physical test device was available and it contained the active test wallet. The team intentionally did not claim a PASS based only on source inspection.

Before v1.0.0 production release, perform the destructive test with a disposable/test wallet or a second device and record:

1. Factory reset completion.
2. Probe result `SETUP_REQUIRED + NOT_INITIALIZED`.
3. Desktop presentation of Create + Recovery.
4. Successful Create New Wallet.
5. Confirmation that recovery words are displayed only on hardware.
6. Post-create `INITIALIZED` state.
7. Lock and subsequent normal PIN unlock.
