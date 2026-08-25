# BC2 Cold Wallet v0.51.5 — USB framing diagnostic hardening

- Keeps the isolated binary BC2 USB channel from v0.51.4.
- USB TX now handles valid partial writes and sends the complete frame within a bounded 100 ms transport deadline.
- Human-readable `bc2_probe.py` is versioned as v0.51.5, waits for a complete BC2 frame, decodes fields and payloads, reports foreign bytes, and prints response latency.
- Probe is present both at repository root and under `tools/` to avoid accidentally using an older external copy.
- No wallet, PIN, seed, recovery, signing, or transaction policy was changed.
