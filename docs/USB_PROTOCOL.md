# BC2 USB Protocol

## Transport

The hardware uses a framed binary protocol over the ESP32-S3 USB Serial/JTAG endpoint. Protocol version is currently `1`.

The BC2 protocol endpoint must contain BC2 frames only. ESP-IDF/boot/display log text must not be multiplexed into the same RX/TX byte stream.

## Frame format

A frame starts with a 9-byte header:

| Offset | Size | Field | Description |
|---:|---:|---|---|
| 0 | 3 | Magic | `42 43 32` = ASCII `BC2` |
| 3 | 1 | Version | Protocol version (`0x01`) |
| 4 | 1 | Command | Request command; response sets bit `0x80` |
| 5 | 2 | Sequence | Little-endian request/response correlation ID |
| 7 | 2 | Payload length | Little-endian payload size |
| 9 | N | Payload | Command-specific data, max 512 bytes |

Example request:

```text
42 43 32 01 01 01 00 0b 00 62 63 32 2d 70 65 6e 74 65 73 74
```

This is protocol v1, `PING (0x01)`, sequence 1, 11-byte payload `bc2-pentest`.

A matching response uses command `0x81`, the same sequence, and the echoed payload.

## Relevant commands

| Command | Value | Purpose |
|---|---:|---|
| PING | `0x01` | Transport/parser echo check |
| GET_INFO | `0x02` | Device information |
| GET_STATE | `0x03` | Current hardware state |
| GET_CAPABILITIES | `0x04` | Hardware capabilities |
| DISPLAY_TEST | `0x10` | Display diagnostic |
| BUTTON_TEST | `0x11` | Button diagnostic |
| REVIEW_RECEIVE_ADDRESS | `0x20` | Hardware receive-address review |
| REVIEW_TRANSACTION | `0x21` | Hardware transaction review |
| GET_TRANSACTION_RESULT | `0x22` | Review result |
| SIGN_TRANSACTION | `0x23` | Signing request |
| GET_SIGN_RESULT | `0x24` | Signing result |
| GET_WALLET_STATUS | `0x40` | Wallet initialization status |
| BEGIN_CREATE_WALLET | `0x41` | Begin wallet creation |
| BEGIN_RECEIVE_ADDRESS | `0x42` | Begin receive-address workflow |
| GET_RECEIVE_RESULT | `0x43` | Receive result |
| BEGIN_RECOVERY | `0x44` | Begin recovery |
| BEGIN_UNLOCK | `0x45` | Begin PIN unlock |
| SUBMIT_RECOVERY_MNEMONIC | `0x46` | Submit recovery mnemonic to hardware |
| LOCK_WALLET | `0x47` | Lock the active wallet/device session |
| GET_WALLET_ID | `0x48` | Obtain wallet identity for public-data isolation |

Responses use the corresponding command with response bit `0x80` set.

## v0.51.3-v0.51.5 isolation work

Internal testing originally observed ESP-IDF/display log lines mixed with valid `BC2` responses. This could make the desktop parser see corrupted or delayed responses.

Hardening work separated the channels:

```text
UART0              -> ESP-IDF diagnostic logging
USB Serial/JTAG    -> BC2 framed binary protocol
```

Boot/ROM logging on the BC2 endpoint was disabled and USB TX was hardened to complete valid partial writes within a bounded transport deadline.

## Serial-port ownership

Only one process should read the BC2 serial endpoint during a protocol session. During testing, running `idf.py monitor` and the probe on `/dev/ttyACM0` at the same time caused each process to consume different portions of the same frame, producing fragments such as `B`, `C2B`, and timeouts.

For diagnostics:

```bash
fuser -v /dev/ttyACM0
```

Close the desktop application and `idf.py monitor` before running the standalone protocol probe. The hardened probe uses exclusive serial access on Linux.
