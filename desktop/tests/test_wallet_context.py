import unittest

from bc2.wallet_context import WalletContext


WALLET_A = "00112233445566778899aabbccddeeff"
WALLET_B = "ffeeddccbbaa99887766554433221100"


class WalletContextTests(unittest.TestCase):
    def test_starts_inactive(self):
        self.assertIsNone(WalletContext().active_wallet_id())

    def test_activate_sets_wallet_id(self):
        context = WalletContext()
        context.activate(WALLET_A)
        self.assertEqual(context.active_wallet_id(), WALLET_A)

    def test_activate_normalizes_case_and_whitespace(self):
        context = WalletContext()
        context.activate(f"  {WALLET_A.upper()}  ")
        self.assertEqual(context.active_wallet_id(), WALLET_A)

    def test_activate_replaces_previous_identity(self):
        context = WalletContext()
        context.activate(WALLET_A)
        context.activate(WALLET_B)
        self.assertEqual(context.active_wallet_id(), WALLET_B)

    def test_deactivate_forgets_identity(self):
        context = WalletContext()
        context.activate(WALLET_A)
        context.deactivate()
        self.assertIsNone(context.active_wallet_id())

    def test_invalid_wallet_id_is_rejected_without_changing_context(self):
        context = WalletContext()
        context.activate(WALLET_A)
        for invalid in ("", "abc", "g" * 32, "00" * 17):
            with self.assertRaises((TypeError, ValueError)):
                context.activate(invalid)
            self.assertEqual(context.active_wallet_id(), WALLET_A)

    def test_non_string_wallet_id_is_rejected(self):
        context = WalletContext()
        with self.assertRaises(TypeError):
            context.activate(None)  # type: ignore[arg-type]
        self.assertIsNone(context.active_wallet_id())


if __name__ == "__main__":
    unittest.main()
