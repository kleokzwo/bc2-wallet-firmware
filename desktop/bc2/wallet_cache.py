from __future__ import annotations

import json
import os
from dataclasses import asdict, dataclass, is_dataclass
from pathlib import Path
from typing import Iterable, Mapping

from bc2.wallet_context import WalletContext


@dataclass(frozen=True)
class WalletCacheData:
    """Public, non-authoritative cached data for exactly one wallet identity."""

    receive_addresses: tuple[str, ...] = ()
    confirmed_balance: int = 0
    unconfirmed_balance: int = 0
    transactions: tuple[dict, ...] = ()
    last_sync: str | None = None


class WalletCache:
    """Small wallet-ID isolated cache for public UX/performance data only.

    Security boundary:
    - every read/write requires an explicit wallet_id
    - each wallet gets its own directory and cache.json file
    - no API exists for seed, mnemonic, PIN, private keys or signing secrets
    - cache corruption is treated as a cache miss, never as wallet authority
    """

    CACHE_VERSION = 1

    def __init__(self, root_dir: str | Path) -> None:
        self._root_dir = Path(root_dir).expanduser()

    def load(self, wallet_id: str) -> WalletCacheData:
        path = self._cache_path(wallet_id)
        try:
            raw = json.loads(path.read_text(encoding="utf-8"))
        except (FileNotFoundError, OSError, UnicodeDecodeError, json.JSONDecodeError):
            return WalletCacheData()

        if not isinstance(raw, dict) or raw.get("version") != self.CACHE_VERSION:
            return WalletCacheData()

        data = raw.get("data")
        if not isinstance(data, dict):
            return WalletCacheData()

        try:
            addresses = self._normalize_addresses(data.get("receive_addresses", []))
            confirmed = self._normalize_amount(data.get("confirmed_balance", 0))
            unconfirmed = self._normalize_amount(data.get("unconfirmed_balance", 0), allow_negative=True)
            transactions = self._normalize_transactions(data.get("transactions", []))
            last_sync = self._normalize_last_sync(data.get("last_sync"))
        except (TypeError, ValueError):
            # A malformed cache must never crash or leak data from another wallet.
            return WalletCacheData()

        return WalletCacheData(
            receive_addresses=tuple(addresses),
            confirmed_balance=confirmed,
            unconfirmed_balance=unconfirmed,
            transactions=tuple(transactions),
            last_sync=last_sync,
        )

    def save_receive_addresses(self, wallet_id: str, addresses: Iterable[str]) -> None:
        data = self.load(wallet_id)
        self._write(wallet_id, WalletCacheData(
            receive_addresses=tuple(self._normalize_addresses(addresses)),
            confirmed_balance=data.confirmed_balance,
            unconfirmed_balance=data.unconfirmed_balance,
            transactions=data.transactions,
            last_sync=data.last_sync,
        ))

    def save_balance(self, wallet_id: str, confirmed: int, unconfirmed: int) -> None:
        data = self.load(wallet_id)
        self._write(wallet_id, WalletCacheData(
            receive_addresses=data.receive_addresses,
            confirmed_balance=self._normalize_amount(confirmed),
            unconfirmed_balance=self._normalize_amount(unconfirmed, allow_negative=True),
            transactions=data.transactions,
            last_sync=data.last_sync,
        ))

    def save_transactions(self, wallet_id: str, transactions: Iterable[object]) -> None:
        data = self.load(wallet_id)
        self._write(wallet_id, WalletCacheData(
            receive_addresses=data.receive_addresses,
            confirmed_balance=data.confirmed_balance,
            unconfirmed_balance=data.unconfirmed_balance,
            transactions=tuple(self._normalize_transactions(transactions)),
            last_sync=data.last_sync,
        ))

    def save_last_sync(self, wallet_id: str, last_sync: str | None) -> None:
        data = self.load(wallet_id)
        self._write(wallet_id, WalletCacheData(
            receive_addresses=data.receive_addresses,
            confirmed_balance=data.confirmed_balance,
            unconfirmed_balance=data.unconfirmed_balance,
            transactions=data.transactions,
            last_sync=self._normalize_last_sync(last_sync),
        ))

    def clear_wallet(self, wallet_id: str) -> None:
        path = self._cache_path(wallet_id)
        try:
            path.unlink()
        except FileNotFoundError:
            return

    def _cache_path(self, wallet_id: str) -> Path:
        normalized = WalletContext._normalize(wallet_id)
        return self._root_dir / normalized / "cache.json"

    def _write(self, wallet_id: str, data: WalletCacheData) -> None:
        path = self._cache_path(wallet_id)
        path.parent.mkdir(parents=True, exist_ok=True)
        try:
            os.chmod(path.parent, 0o700)
        except OSError:
            pass

        payload = {
            "version": self.CACHE_VERSION,
            "data": {
                "receive_addresses": list(data.receive_addresses),
                "confirmed_balance": data.confirmed_balance,
                "unconfirmed_balance": data.unconfirmed_balance,
                "transactions": list(data.transactions),
                "last_sync": data.last_sync,
            },
        }

        temporary = path.with_suffix(".tmp")
        temporary.write_text(
            json.dumps(payload, ensure_ascii=False, separators=(",", ":")),
            encoding="utf-8",
        )
        try:
            os.chmod(temporary, 0o600)
        except OSError:
            pass
        os.replace(temporary, path)

    @staticmethod
    def _normalize_addresses(addresses: Iterable[str]) -> list[str]:
        if isinstance(addresses, (str, bytes)):
            addresses = [addresses.decode() if isinstance(addresses, bytes) else addresses]
        result: list[str] = []
        seen: set[str] = set()
        for address in addresses:
            value = str(address).strip()
            if value and value not in seen:
                seen.add(value)
                result.append(value)
        return result

    @staticmethod
    def _normalize_amount(value: object, *, allow_negative: bool = False) -> int:
        if isinstance(value, bool) or not isinstance(value, int):
            raise TypeError("balance must be an integer number of satoshis")
        if not allow_negative and value < 0:
            raise ValueError("confirmed balance cannot be negative")
        return value

    @classmethod
    def _normalize_transactions(cls, transactions: Iterable[object]) -> list[dict]:
        if isinstance(transactions, (str, bytes, Mapping)):
            raise TypeError("transactions must be an iterable of transaction records")

        result: list[dict] = []
        for item in transactions:
            if is_dataclass(item):
                item = asdict(item)
            if not isinstance(item, Mapping):
                raise TypeError("transaction record must be a mapping or dataclass")

            txid = str(item.get("txid", "")).strip().lower()
            if len(txid) != 64:
                raise ValueError("transaction txid must contain exactly 64 hex characters")
            try:
                bytes.fromhex(txid)
            except ValueError as exc:
                raise ValueError("transaction txid must be hexadecimal") from exc

            direction = str(item.get("direction", "")).strip().lower()
            if direction not in {"incoming", "outgoing", "self"}:
                raise ValueError(
                    "transaction direction must be incoming, outgoing or self"
                )

            amount = item.get("amount")
            height = item.get("height")
            if isinstance(amount, bool) or not isinstance(amount, int) or amount < 0:
                raise ValueError("transaction amount must be a non-negative integer")
            if isinstance(height, bool) or not isinstance(height, int):
                raise ValueError("transaction height must be an integer")

            result.append({
                "txid": txid,
                "direction": direction,
                "amount": amount,
                "height": height,
            })
        return result

    @staticmethod
    def _normalize_last_sync(value: object) -> str | None:
        if value is None:
            return None
        if not isinstance(value, str):
            raise TypeError("last_sync must be a string or None")
        value = value.strip()
        return value or None
