# Sprint 9 CI Hotfix 6 Report — v0.17.7

## Problem

The Windows compiler build and all 13 tests completed successfully, but CPack failed during Qt runtime deployment.

Qt reported that parts of the executable name were unsupported arguments. The generated deployment script received the executable name `BC2 Cold Wallet.exe`; its spaces were interpreted as separate list arguments by the deployment helper.

## Fix

The Windows executable output name is now:

```text
BC2-Cold-Wallet.exe
```

The human-facing application name and macOS bundle name remain `BC2 Cold Wallet`.

## Scope

No wallet-core, cryptographic, seed, signing, Electrum, USB, hardware, or device-state logic was changed.
