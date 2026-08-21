from pathlib import Path


def _source() -> str:
    return (
        Path(__file__).resolve().parents[1]
        / "bc2"
        / "ui"
        / "main_window.py"
    ).read_text(encoding="utf-8")


def test_cache_is_rendered_before_background_sync_is_queued():
    source = _source()
    load = source.index("self._load_active_wallet_cache()")
    navigate = source.index('self._navigate("dashboard")', load)
    sync = source.index("QTimer.singleShot(0, self._sync_balance)", navigate)
    assert load < navigate < sync


def test_cached_transactions_are_not_blank_during_background_sync():
    source = _source()
    start = source.index("def _on_transaction_sync_started")
    end = source.index("def ", start + 4)
    body = source[start:end]
    assert "if not self._transaction_entries:" in body
    assert "self._transaction_page.show_loading()" in body


def test_network_is_not_claimed_connected_from_cache():
    source = _source()
    start = source.index("def _load_active_wallet_cache")
    end = source.index("def ", start + 4)
    body = source[start:end]
    assert 'set_network_state("Nicht verbunden")' in body
    assert 'set_network_state("Verbunden")' not in body


def test_sync_failure_keeps_existing_cached_transactions():
    source = _source()
    start = source.index("def _on_transaction_sync_failed")
    end = source.index("def ", start + 4)
    body = source[start:end]
    assert "if not self._transaction_entries:" in body
