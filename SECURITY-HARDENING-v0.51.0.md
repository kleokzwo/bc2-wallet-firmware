# BC2 Cold Wallet — Security Hardening v0.51.0

## Scope

Static review and conservative cleanup of the supplied BC2-only cold-wallet source tree. This is a source-level hardening pass, not a claim of a completed professional hardware penetration test.

## Changes applied

- Removed generated ESP-IDF/CMake build trees and Python cache directories from the release tree.
- Added a root `.gitignore` for build, cache, editor, log and temporary artifacts.
- Removed the obsolete desktop inactivity/countdown test after the product policy intentionally removed that feature.
- Removed unused PySide6 imports.
- Reduced recovery-mnemonic lifetime in the desktop UI by adding one-shot `take_mnemonic()` semantics.
- Centralized hardware `encrypted entropy -> mnemonic -> BIP39 seed -> BIP32 master` derivation in one internal helper with a single cleanup/zeroization path.
- Made the mnemonic/xprv developer CLI opt-in (`BC2_BUILD_DEVELOPER_CLI=OFF` by default), so release builds do not produce a tool that prints private extended keys.
- Enforced the product's USB-only policy by disabling Wi-Fi in ESP-IDF defaults/current config; Bluetooth remains disabled.

## Validation after cleanup

- Desktop tests: 67 passed, 1 skipped (PySide6 UI smoke test unavailable in the review environment).
- Host C17/core CMake build: successful with warnings-as-errors.
- Host CTest: 21/21 passed.

## Security findings

### RELEASE BLOCKER — Secure Boot and Flash Encryption are disabled

The current ESP32-S3 `sdkconfig` has hardware Secure Boot disabled and flash encryption disabled. The wallet seed is wrapped using a key derived through an eFuse HMAC key, which is a strong design primitive, but it does not by itself establish firmware authenticity. A physical attacker who can replace firmware may be able to execute code that invokes permitted hardware cryptographic operations and targets wallet secrets.

Production devices must use **Secure Boot v2 together with Flash Encryption in Release mode** and an appropriate ROM download/JTAG policy. This must be provisioned as a separate production/manufacturing step because enabling these features burns security eFuses and can permanently change reflashing/debug behavior.

**Do not enable/burn production security eFuses on a development device until the provisioning procedure and key custody are finalized and tested on a sacrificial device.**

### HIGH — Physical/debug interface policy needs production provisioning

USB Serial/JTAG is used by the current board/protocol path. Production provisioning must explicitly decide which ROM download/debug paths remain available after manufacturing. Development convenience settings must not silently ship as the production threat model.

### MEDIUM — Desktop recovery necessarily exposes the mnemonic to the host

This is an intentional BC2 product decision: recovery words are entered in the desktop app and sent to hardware. The desktop does not persist/log them, and this pass reduces retained references, but Python strings cannot be reliably zeroized in-place. Recovery therefore trusts the desktop host for the duration of recovery. The UI warning should remain.

### MEDIUM — Legacy migration code remains

The wallet-specific receive-address migration path remains because it may be needed to upgrade existing installations safely. It should be removed after the v1.0 migration window if backward compatibility is no longer required.

## Production gate before v1.0

1. Finalize Secure Boot v2 signing-key custody and recovery procedure.
2. Finalize ESP32-S3 Flash Encryption **Release** provisioning.
3. Decide and test JTAG / ROM-download lock policy on a sacrificial board.
4. Rebuild from clean source using the production security configuration.
5. Verify eFuse state on the produced device.
6. Re-run the complete create/backup/unlock/receive/send/lock/lockdown/recovery test matrix on real hardware.
7. Attempt hostile USB/state-machine sequences and malformed protocol frames against the release firmware.
8. Only then tag the firmware as BC2 Cold Wallet v1.0.
