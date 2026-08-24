# Troubleshooting

## Hardware wallet is not detected

- Confirm the USB cable supports data.
- Disconnect/reconnect the device.
- Ensure another application is not holding the serial endpoint.
- On Linux, inspect ownership with:

```bash
fuser -v /dev/ttyACM0
```

## Protocol probe shows partial frames or timeouts

Do not run multiple readers against the same BC2 serial endpoint. In particular, close:

- BC2 desktop application;
- `idf.py monitor`;
- other serial terminals;
- other probe processes.

Then run the probe alone. The exclusive probe variant prevents another normal Linux process from opening the same tty concurrently.

During development, simultaneous `idf.py monitor` and probe access was proven to split BC2 frames between the two processes and create misleading fragments/timeouts.

## `device reports readiness to read but returned no data`

This pyserial message can occur when the USB serial connection is reset/re-enumerated or when multiple processes compete for the port. First eliminate multiple-access conditions before treating it as a firmware defect.

## Device says LOCKDOWN / Recovery required

LOCKDOWN is expected after the third consecutive incorrect PIN. Use the official Recovery workflow with the correct recovery words.

## New device does not show Create New Wallet

Expected source-defined behavior for a truly empty device is `SETUP_REQUIRED + NOT_INITIALIZED`, with Create New Wallet and Recovery Wallet visible. The physical fresh-device release test is still pending. If a device reports `INITIALIZED`, it is not in factory state.

## Never debug with real secrets

Development tests, logs, screenshots, probes, and destructive reset tests should use disposable test wallets. Do not paste real PINs, recovery words, private keys, or funded-wallet secrets into issue reports.
