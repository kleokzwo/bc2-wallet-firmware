# BC2 Cold Wallet v0.50.0 — Sprint S1 Hardware Security State Machine

## Binding security model

Normal hardware lifecycle:

FACTORY -> Create/Recovery -> LOCKED -> PIN -> UNLOCKED

Three wrong device-PIN verifications enter persistent LOCKDOWN. In LOCKDOWN normal unlock and wallet operations are forbidden. Recovery is the only recovery path.

## Implemented in S1

- Explicit `BC2_DEVICE_LOCKDOWN` state.
- Explicit `BC2_DEVICE_EVENT_ENTER_LOCKDOWN` transition.
- Device PIN failure limit reduced from 10/5-at-state-machine to exactly 3.
- Third wrong PIN returns `BC2_PIN_SECURITY_LOCKED` and persists failure count in the existing PIN record.
- No extra lockdown database/record was added. The existing minimal PIN security record is the source of truth.
- Reboot reloads the persisted failure count and re-enters hardware LOCKDOWN.
- Correct old PIN is rejected once LOCKDOWN is active.
- Receive/transaction authorization PIN failures also respect the same global three-attempt security boundary.
- Hardware wallet ID is cleared when entering lockdown.
- Hardware renders `RECOVERY ERFORDERLICH` in lockdown.
- Recovery request is permitted from LOCKDOWN and skips the unusable old PIN; a new PIN is created as part of recovery.
- Idle/session timeout remains a normal LOCKED transition and does not increment PIN failures.

## Intentionally NOT part of S1

- Desktop button policy (Create hidden after first setup) — Sprint S2.
- Removal of Replace-Create path — Sprint S2.
- Final LOCKED-vs-LOCKDOWN Recovery desktop UX — later sprint.
- Desktop 15 min inactivity + 60 sec countdown — separate Desktop Session Guard sprint.

## Automated verification

Portable firmware C/C++ suite: 21/21 passed with `-Wall -Wextra -Wpedantic -Werror`.
Desktop pytest suite: 52 passed, 1 skipped, 0 failed.

Hardware ESP-IDF build/flash and physical acceptance still required on the Waveshare ESP32-S3.
