# Security Policy

## Scope

BC2 Cold Wallet handles cryptographic wallet material and should be treated as security-sensitive software. Reports concerning secret exposure, signing authorization, PIN/lockdown bypass, recovery handling, wallet isolation, USB protocol parsing, or transaction/address confirmation are security-relevant.

## Reporting a vulnerability

Do not publish real recovery phrases, PINs, private keys, or funded-wallet data in a public issue. Provide a minimal reproduction using disposable test data and clearly identify the affected firmware/desktop version and hardware revision.

Until a dedicated private disclosure channel is published by the project owner, avoid including exploitable secret material in public reports. Project maintainers should establish a private security contact before production launch.

## Security architecture summary

- Hardware is the authority for wallet secrets and security state.
- Device PIN is exactly four digits.
- Three failed unlock attempts lead to LOCKDOWN.
- Newly created recovery words are generated/displayed on hardware.
- Recovery is desktop-assisted; the desktop must not persist or log the recovery mnemonic.
- Receive addresses and transaction details are intended to be confirmed on hardware.
- Desktop public cache is isolated by wallet identity and is non-authoritative.
- BC2 binary USB protocol traffic is isolated from ESP-IDF diagnostic logging.

See `docs/SECURITY_MODEL.md` and `docs/SECURITY_TESTS.md` for details and current verification status.

## Audit status

The repository contains internal security, protocol, and integration test records. These must not be represented as an independent security audit, certification, or third-party penetration test.
