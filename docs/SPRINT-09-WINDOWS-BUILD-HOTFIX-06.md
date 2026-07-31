# BC2 Cold Wallet v0.17.6

## Windows Qt compile hotfix

### Scope

This hotfix addresses only compiler errors reported by the native Windows GitHub Actions build.

### Fixes

1. `mainwindow_pages.cpp` now includes `QDesktopServices`.
2. The BC2 explorer base URL is defined in the same translation unit where it is used.
3. The local page-change boolean in `PageRouter::show()` was renamed to `hasPageChanged`, preventing a name collision with the `pageChanged(Page)` Qt signal under MSVC.

### Security impact

No wallet-core, seed, private-key, PIN, signing, network, USB, or hardware behavior changed.

### Verification

The host core and all tests must remain green. The native Qt/MSVC build is verified by the next GitHub Actions run.
