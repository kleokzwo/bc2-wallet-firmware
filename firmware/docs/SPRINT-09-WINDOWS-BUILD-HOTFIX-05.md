# Windows Desktop Build Hotfix 05

## Version

BC2 Cold Wallet v0.17.5

## Problem

The Windows CI reached the native MSVC build, but release creation was still blocked by compiler warnings promoted to errors through `/WX`. Cross-platform warning sets differ between GCC, Clang, MSVC, Qt, and OpenSSL. This prevented producing a testable Windows desktop package.

## Binding decision

- The Windows core, CLI, and tests use MSVC warning level `/W4`.
- `/WX` is not used for the Windows release job.
- Assertions remain enabled in Windows tests through `/UNDEBUG`.
- Linux and macOS retain the existing strict warning configuration.
- CI build output is verbose so the exact compiler command and error remain visible.

## Security boundary

This change does not alter wallet behavior, cryptographic calculations, seed handling, signing, networking, USB, HAL, or hardware code. It changes only compiler policy for the Windows desktop release.

## Definition of Done

- Host core and CLI compile.
- All host tests pass.
- Windows workflow reaches package creation without warnings stopping compilation.
- Repository ZIP and SHA-256 file are produced.
