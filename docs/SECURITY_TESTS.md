# Internal Security and Protocol Test Record

**Test date:** 2026-08-24  
**Hardware:** Waveshare ESP32-S3, 1.54-inch 200x200 e-paper, board revision 2  
**Firmware line:** v0.51.3-v0.51.5 USB/protocol hardening  
**Host:** Linux / `/dev/ttyACM0`  
**Method:** physical device testing plus read-only BC2 protocol probe for PING, GET_STATE and GET_WALLET_STATUS.

This document records internal development tests. It is not evidence of an independent audit, certification, or third-party penetration test.

## Result summary

| Test | Result | Evidence / observation |
|---|---|---|
| USB binary-channel isolation | PASS | ESP-IDF/display text no longer appeared inside BC2 responses after hardening |
| PING framing | PASS | Correct `0x01 -> 0x81`, matching sequence and `bc2-pentest` echo |
| GET_STATE framing | PASS | Correct `0x03 -> 0x83`, matching sequence |
| GET_WALLET_STATUS framing | PASS | Correct `0x40 -> 0xC0`, matching sequence |
| Response latency sanity | PASS | Typical observed responses approximately 2-20 ms in successful exclusive-port runs |
| Exclusive serial ownership diagnosis | PASS | Concurrent `idf.py monitor` reproduced split/partial frames; exclusive probe removed interference |
| Initialized wallet, locked state | PASS | `GET_STATE=0x02 LOCKED`, wallet status `0x02 INITIALIZED` |
| Correct PIN unlock | PASS | State changed `LOCKED (0x02) -> DASHBOARD (0x05)`; wallet remained initialized |
| Automatic hardware lock | PASS | After idle timeout, state changed `DASHBOARD -> LOCKED`; wallet remained initialized |
| Wrong PIN attempt 1 | PASS | Device remained `LOCKED`; wallet remained initialized |
| Wrong PIN attempt 2 | PASS | Device remained `LOCKED`; wallet remained initialized |
| Wrong PIN attempt 3 | PASS | Device entered `LOCKDOWN (0x0A)`; wallet remained initialized |
| LOCKDOWN -> Recovery -> new PIN -> DASHBOARD | NOT TESTED | Test cycle not completed in this session |
| Fresh/factory device state | NOT TESTED | Only one physical device available; active test wallet was retained |
| Fresh-device Create New Wallet UX | NOT TESTED | Depends on factory-state test above |

## Detailed observations

### Clean protocol framing

Successful PING example:

```text
TX command    = 0x01 PING
RX command    = 0x81 RESPONSE PING
Sequence      = 1 / 1
Payload       = bc2-pentest
Magic         = OK
Version       = OK
Command       = OK
Sequence      = OK
```

The same request/response validation passed for GET_STATE and GET_WALLET_STATUS.

### Serial-port contention finding

An apparent protocol corruption was reproduced while `idf.py monitor` and the Python probe accessed `/dev/ttyACM0` concurrently. Monitor output contained BC2 bytes and the probe received partial prefixes such as `C2B` or incomplete frames. This was diagnosed as competing readers, not a valid BC2 frame produced by the firmware.

Test rule established: desktop, monitor, and standalone probe must not concurrently read the same BC2 serial endpoint.

### Normal locked state

Observed:

```text
Device-State : 0x02 = LOCKED
Wallet-Status: 0x02 = INITIALIZED
```

### Correct PIN

After entering the correct 4-digit PIN:

```text
Device-State : 0x05 = DASHBOARD
Wallet-Status: 0x02 = INITIALIZED
```

### Automatic lock

After allowing the hardware to enter its own idle/locked state:

```text
Device-State : 0x02 = LOCKED
Wallet-Status: 0x02 = INITIALIZED
```

This directly verified the hardware automatic lock behavior.

### Three-attempt PIN policy

After wrong PIN attempt 1:

```text
Device-State : LOCKED
Wallet-Status: INITIALIZED
```

After wrong PIN attempt 2:

```text
Device-State : LOCKED
Wallet-Status: INITIALIZED
```

After wrong PIN attempt 3:

```text
Device-State : 0x0A = LOCKDOWN
Wallet-Status: 0x02 = INITIALIZED
```

This confirms the firmware transition into LOCKDOWN after the third failed attempt without reporting the wallet as uninitialized.

## Pending release tests

The following must remain visibly pending until executed:

1. Complete LOCKDOWN recovery using a test mnemonic, create a new 4-digit PIN, and verify `DASHBOARD + INITIALIZED`.
2. Verify an actual fresh/factory device reports `SETUP_REQUIRED (0x01) + NOT_INITIALIZED (0x00)`.
3. On that fresh state, verify the desktop presents `Create New Wallet` and `Recovery Wallet` and hides normal Unlock.
4. Complete fresh Create New Wallet lifecycle and verify the seed is shown only on hardware.
5. Run final Receive and Send/Review/Sign/Broadcast end-to-end smoke tests with test funds.
6. Reconfirm wallet-ID cache isolation after wallet replacement/recovery.

Do not convert a `NOT TESTED` entry to `PASS` based only on source inspection.
