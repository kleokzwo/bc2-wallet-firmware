from pathlib import Path


def _main_source() -> str:
    return (
        Path(__file__).resolve().parents[1]
        / "bc2"
        / "ui"
        / "main_window.py"
    ).read_text(encoding="utf-8")


def test_legacy_addresses_are_never_automatically_assigned():
    source = _main_source()
    assert "_offer_legacy_receive_address_migration" in source
    assert "QMessageBox.question" in source
    assert "QMessageBox.No" in source


def test_legacy_global_key_is_removed_only_after_confirmed_migration():
    source = _main_source()
    save_pos = source.index("_wallet_cache.save_receive_addresses(wallet_id, legacy)")
    remove_pos = source.index('_settings.remove("wallet/receive_addresses")')
    assert save_pos < remove_pos
