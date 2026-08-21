from pathlib import Path


def test_cached_receive_address_is_never_restored_into_receive_ui():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "ui" / "main_window.py"
    ).read_text(encoding="utf-8")

    start = source.index("def _load_active_wallet_cache")
    end = source.index("def ", start + 4)
    body = source[start:end]

    assert "show_cached_address" not in body
    assert "reset_wallet_view()" in body
