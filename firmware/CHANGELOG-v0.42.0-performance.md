# BC2 Cold Wallet Hardware v0.42.0 – Performance & Readability Pass

## Scope

Small, low-risk firmware pass for the existing Waveshare ESP32-S3 1.54-inch E-Paper hardware. No wallet/security architecture rewrite.

## Changes

- ESP32-S3 default CPU clock raised from 160 MHz to 240 MHz.
- Main application poll interval reduced from 50 ms to 10 ms for more responsive button/USB state handling.
- Display adapter no longer forces a full E-Paper refresh merely because the screen title changed.
- Receive review and transaction review use the fast partial-refresh path; explicit full refreshes and the periodic ghosting guard remain in place.
- Existing ghosting protection retained: after 12 partial refreshes, a full refresh is forced.
- Added display performance logging with render, refresh and total time in milliseconds.
- Added the supplied BC2-II logo as a 32x32 monochrome status-bar bitmap.
- Added adaptive larger headings: titles up to 16 characters render at 2x scale; longer titles remain 1x to avoid clipping.

## Runtime log example

`I (...) bc2_display: BC2 frame partial: render=...ms refresh=...ms total=...ms partial_count=...`

Use these values to decide whether further optimization should target rendering, E-Paper refresh, or another subsystem.

## Safety

- Seed/private-key/PIN handling unchanged.
- USB protocol unchanged.
- NVS wallet storage unchanged.
- Signing logic unchanged.
- Full-refresh escape hatch remains available via `require_full_refresh`.
