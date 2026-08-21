# BC2 Desktop v0.47.0 — Phase 3 WalletCache

- Added a small stdlib-only `WalletCache` for public UX/performance data.
- Cache storage is strictly isolated by the 128-bit `wallet_id` contract from Phase 1.
- Each wallet gets its own `<cache-root>/<wallet_id>/cache.json` file.
- Supported cached data: receive addresses, confirmed/unconfirmed balance, transactions and `last_sync`.
- No API exists for seed, mnemonic, PIN, private keys or signing secrets.
- Malformed/corrupted cache files are treated as an empty cache and never crash the wallet.
- Writes are atomic via temporary file + `os.replace`.
- Added a separate hardware isolation probe for an A -> B -> A test without touching current production QSettings data.
- No MainWindow cache migration, instant dashboard or background-sync behavior was added yet; those remain later phases.
