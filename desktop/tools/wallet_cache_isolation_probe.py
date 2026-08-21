#!/usr/bin/env python3
"""Phase-3 hardware/cache isolation probe.

This intentionally uses a dedicated diagnostic cache directory and does NOT
read or modify the application's current QSettings wallet data.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

from bc2.device.discovery import get_wallet_id
from bc2.wallet_cache import WalletCache


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="/dev/ttyACM0")
    parser.add_argument(
        "--cache-dir",
        default=str(Path.home() / ".cache" / "bc2-wallet-phase3-test"),
    )
    parser.add_argument(
        "--write-marker",
        type=int,
        help="Store a synthetic confirmed-balance marker for the currently unlocked wallet.",
    )
    args = parser.parse_args()

    wallet_id = get_wallet_id(args.port)
    if wallet_id is None:
        print("ERROR: Keine Wallet-ID. Gerät zuerst entsperren und Desktop/monitor schließen.")
        return 2

    cache = WalletCache(args.cache_dir)
    if args.write_marker is not None:
        if args.write_marker < 0:
            print("ERROR: Marker muss >= 0 sein.")
            return 2
        cache.save_balance(wallet_id, args.write_marker, 0)

    data = cache.load(wallet_id)
    print(f"Wallet-ID : {wallet_id}")
    print(f"Marker    : {data.confirmed_balance}")
    print(f"Cache     : {Path(args.cache_dir).expanduser() / wallet_id / 'cache.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
