# BC2 Cold Wallet Documentation

This directory is the documentation hub for the BC2 Cold Wallet repository. It separates end-user guidance from developer/security documentation and records which release checks have actually been executed.

## User documentation

- [Getting Started](GETTING_STARTED.md)
- [Community HOWTO](COMMUNITY_HOWTO.md)
- [User Guide](USER_GUIDE.md)
- [PIN and Lockdown](PIN_AND_LOCKDOWN.md)
- [Recovery](RECOVERY.md)
- [Factory Reset](FACTORY_RESET.md)
- [Troubleshooting](TROUBLESHOOTING.md)

## Developer and security documentation

- [Architecture](ARCHITECTURE.md)
- [Security Model](SECURITY_MODEL.md)
- [Device States](DEVICE_STATES.md)
- [USB Protocol](USB_PROTOCOL.md)
- [Internal Security Test Record](SECURITY_TESTS.md)
- [v1.0.0 Release Checklist](RELEASE_CHECKLIST.md)
- [Desktop Translations](TRANSLATIONS.md)

## Documentation status

The security test record distinguishes `PASS`, `FAIL`, and `NOT TESTED`. A feature described by source code is not automatically marked as tested. In particular, the fresh-device/factory-state lifecycle remains pending because only one physical test device was available during the 2026-08-24 verification session.

These records describe internal development and integration testing. They are not a claim of an independent security audit, certification, or third-party penetration test.
