# v1.0.0 Release Checklist

This checklist is the release gate for a production v1.0.0 decision. `PASS` means the behavior was actually exercised; source inspection alone does not count as PASS.

## Protocol and hardware state

- [x] PASS — BC2 USB channel isolated from ESP-IDF/display log text.
- [x] PASS — PING request/response framing.
- [x] PASS — GET_STATE request/response framing.
- [x] PASS — GET_WALLET_STATUS request/response framing.
- [x] PASS — Command response bit and sequence matching.
- [x] PASS — Standalone probe works with exclusive serial ownership.
- [x] PASS — Normal initialized/locked state reports `LOCKED + INITIALIZED`.
- [x] PASS — Correct PIN transitions `LOCKED -> DASHBOARD`.
- [x] PASS — Hardware automatic timeout transitions `DASHBOARD -> LOCKED`.

## PIN and lockdown

- [x] PASS — Wrong PIN #1 remains LOCKED.
- [x] PASS — Wrong PIN #2 remains LOCKED.
- [x] PASS — Wrong PIN #3 enters LOCKDOWN.
- [x] PASS — Wallet remains INITIALIZED when LOCKDOWN is entered.
- [ ] TODO — Complete `LOCKDOWN -> Recovery -> new 4-digit PIN -> DASHBOARD` release test.

## Fresh-device lifecycle

- [ ] TODO — Destructive/second-device factory-state test.
- [ ] TODO — Verify `SETUP_REQUIRED (0x01) + NOT_INITIALIZED (0x00)` on real fresh state.
- [ ] TODO — Verify desktop shows Create New Wallet + Recovery Wallet and hides Unlock.
- [ ] TODO — Complete Create New Wallet from fresh state.
- [ ] TODO — Verify newly generated recovery words are displayed only on hardware.
- [ ] TODO — Lock/reboot/reconnect and unlock newly created test wallet.

## End-to-end wallet functions

These workflows have been exercised during development, but perform a final release-candidate smoke run on the exact production build:

- [ ] TODO — Receive address request and hardware confirmation.
- [ ] TODO — Send: prepare -> hardware review -> sign -> broadcast with test funds.
- [ ] TODO — Transaction history/balance refresh.
- [ ] TODO — Logout/lock and subsequent unlock.
- [ ] TODO — USB disconnect/reconnect recovery.
- [ ] TODO — Wallet replacement/recovery does not expose another wallet's public cache, receive address, or prepared transaction.

## Production hygiene

- [ ] TODO — Clean reproducible production build from source (`fullclean`).
- [ ] TODO — Confirm production firmware/version metadata is correct.
- [ ] TODO — Confirm no test mnemonic, PIN, private key, or funded-wallet secret exists in repository/build artifacts.
- [ ] TODO — Review release archive contents; exclude unnecessary build/debug artifacts.
- [ ] TODO — Review user documentation and recovery/reset warnings.
- [ ] TODO — Record final release test date, hardware revision, firmware hash/tag, and desktop version.
- [ ] TODO — Tag v1.0.0 only after all blocking items are accepted.

## Current decision

As of 2026-08-24: **Release Candidate / NOT YET APPROVED AS v1.0.0 production release.**

Reason: core USB/state/PIN-lockdown tests passed, but the full LOCKDOWN recovery cycle and physical fresh-device Create lifecycle remain unverified in this release-gate session.
