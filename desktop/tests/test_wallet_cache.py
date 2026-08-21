import json
import tempfile
import unittest
from dataclasses import dataclass
from pathlib import Path

from bc2.wallet_cache import WalletCache, WalletCacheData


WALLET_A = "00112233445566778899aabbccddeeff"
WALLET_B = "ffeeddccbbaa99887766554433221100"
TX_A = "11" * 32
TX_B = "22" * 32


@dataclass(frozen=True)
class FakeTransaction:
    txid: str
    direction: str
    amount: int
    height: int


class WalletCacheTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.cache = WalletCache(self.root)

    def tearDown(self):
        self.temp.cleanup()

    def test_missing_wallet_is_empty(self):
        self.assertEqual(self.cache.load(WALLET_A), WalletCacheData())

    def test_wallets_are_strictly_isolated(self):
        self.cache.save_receive_addresses(WALLET_A, ["bc2-address-a"])
        self.cache.save_balance(WALLET_A, 111, -1)
        self.cache.save_transactions(WALLET_A, [
            {"txid": TX_A, "direction": "incoming", "amount": 111, "height": 10}
        ])
        self.cache.save_last_sync(WALLET_A, "2026-08-20T11:00:00+02:00")

        self.cache.save_receive_addresses(WALLET_B, ["bc2-address-b"])
        self.cache.save_balance(WALLET_B, 222, 0)
        self.cache.save_transactions(WALLET_B, [
            FakeTransaction(TX_B, "outgoing", 222, 0)
        ])

        a = self.cache.load(WALLET_A)
        b = self.cache.load(WALLET_B)

        self.assertEqual(a.receive_addresses, ("bc2-address-a",))
        self.assertEqual(a.confirmed_balance, 111)
        self.assertEqual(a.transactions[0]["txid"], TX_A)
        self.assertEqual(a.last_sync, "2026-08-20T11:00:00+02:00")

        self.assertEqual(b.receive_addresses, ("bc2-address-b",))
        self.assertEqual(b.confirmed_balance, 222)
        self.assertEqual(b.transactions[0]["txid"], TX_B)
        self.assertIsNone(b.last_sync)

    def test_same_wallet_id_recovers_same_cache(self):
        self.cache.save_balance(WALLET_A, 123456789, 0)
        second_instance = WalletCache(self.root)
        self.assertEqual(second_instance.load(WALLET_A).confirmed_balance, 123456789)

    def test_invalid_wallet_id_is_rejected(self):
        for invalid in ("", "abc", "g" * 32, "00" * 17):
            with self.assertRaises((TypeError, ValueError)):
                self.cache.load(invalid)

    def test_corrupted_cache_is_safe_cache_miss(self):
        path = self.root / WALLET_A / "cache.json"
        path.parent.mkdir(parents=True)
        path.write_text("{broken", encoding="utf-8")
        self.assertEqual(self.cache.load(WALLET_A), WalletCacheData())

    def test_malformed_cache_is_safe_cache_miss(self):
        path = self.root / WALLET_A / "cache.json"
        path.parent.mkdir(parents=True)
        path.write_text(json.dumps({
            "version": 1,
            "data": {"confirmed_balance": "not-an-int"},
        }), encoding="utf-8")
        self.assertEqual(self.cache.load(WALLET_A), WalletCacheData())

    def test_clear_wallet_does_not_touch_other_wallet(self):
        self.cache.save_balance(WALLET_A, 111, 0)
        self.cache.save_balance(WALLET_B, 222, 0)
        self.cache.clear_wallet(WALLET_A)
        self.assertEqual(self.cache.load(WALLET_A), WalletCacheData())
        self.assertEqual(self.cache.load(WALLET_B).confirmed_balance, 222)

    def test_transaction_validation_rejects_bad_records(self):
        bad = {"txid": "bad", "direction": "incoming", "amount": 1, "height": 1}
        with self.assertRaises(ValueError):
            self.cache.save_transactions(WALLET_A, [bad])


if __name__ == "__main__":
    unittest.main()
