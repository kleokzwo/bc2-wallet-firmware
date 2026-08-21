from pathlib import Path


def test_main_window_uses_wallet_cache_for_active_receive_addresses():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2"
        / "ui"
        / "main_window.py"
    ).read_text(encoding="utf-8")
    assert "_wallet_cache.save_receive_addresses" in source
    assert "_wallet_context.active_wallet_id()" in source
    # The legacy global key may only exist inside the explicit one-time
    # migration path; normal reads/writes use WalletCache.
    assert 'self._settings.setValue("wallet/receive_addresses"' not in source
    assert "_offer_legacy_receive_address_migration" in source


def test_stale_sync_results_are_wallet_bound():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2"
        / "ui"
        / "main_window.py"
    ).read_text(encoding="utf-8")
    assert "_balance_sync_wallet_id" in source
    assert "_transaction_sync_wallet_id" in source
    assert "request_wallet_id != wallet_id" in source
