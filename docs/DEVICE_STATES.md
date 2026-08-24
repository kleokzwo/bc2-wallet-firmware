# Device State Machine

The firmware exposes the current state through `GET_STATE` (`0x03`). The response payload is one byte containing the state value defined by `bc2_device_state`.

| Value | State | Meaning |
|---:|---|---|
| `0x00` | `BOOT` | Device startup state |
| `0x01` | `SETUP_REQUIRED` | No initialized wallet; setup/recovery required |
| `0x02` | `LOCKED` | Wallet initialized, device locked |
| `0x03` | `UNLOCKING` | Unlock operation in progress |
| `0x04` | `COOLDOWN` | Temporary cooldown state |
| `0x05` | `DASHBOARD` | Wallet unlocked / normal active state |
| `0x06` | `RECEIVE_REVIEW` | Receive-address review on device |
| `0x07` | `TRANSACTION_REVIEW` | Transaction review on device |
| `0x08` | `SETTINGS` | Hardware settings state |
| `0x09` | `ERROR` | Device security/error state |
| `0x0A` | `LOCKDOWN` | Recovery required after security lockout |

## Wallet status

`GET_WALLET_STATUS` (`0x40`) returns one byte:

| Value | Meaning |
|---:|---|
| `0x00` | Wallet not initialized |
| `0x02` | Wallet initialized |

## Important transitions

```text
BOOT
  | wallet absent
  v
SETUP_REQUIRED ---- Create / Recovery ----> DASHBOARD

BOOT
  | wallet present
  v
LOCKED ---- correct PIN ----> DASHBOARD
  ^                            |
  |                            | lock / session timeout
  +----------------------------+

LOCKED -- wrong PIN #1 --> LOCKED
LOCKED -- wrong PIN #2 --> LOCKED
LOCKED -- wrong PIN #3 --> LOCKDOWN

LOCKDOWN -- Recovery + new PIN --> DASHBOARD   [release test still pending]
```

The firmware default maximum unlock attempts is 3. The firmware default unlocked-session timeout is 5 minutes.

## Desktop presentation policy

Expected UI by hardware state:

- `SETUP_REQUIRED` / wallet not initialized: show `Create New Wallet` and `Recovery Wallet`; hide normal Unlock.
- `LOCKED` / wallet initialized: show `Unlock Wallet`; hide Create and Recovery.
- `LOCKDOWN`: show Recovery only.
- `DASHBOARD`: enter the normal wallet UI.
- `ERROR`: do not automatically overwrite wallet state; present a security error.
