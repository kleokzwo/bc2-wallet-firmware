from pathlib import Path


def test_send_page_has_hard_wallet_session_reset():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "ui" / "pages" / "send_page.py"
    ).read_text(encoding="utf-8")

    assert "def reset_wallet_view" in source
    assert "self._address_input.clear()" in source
    assert "self._amount_input.clear()" in source
    assert 'self._preview_text.setText("")' in source


def test_wallet_ui_clear_destroys_send_session():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "ui" / "main_window.py"
    ).read_text(encoding="utf-8")

    clear_start = source.index("def _clear_wallet_ui")
    clear_end = source.index("def ", clear_start + 4)
    clear_body = source[clear_start:clear_end]
    assert "self._reset_send_session()" in clear_body


def test_wallet_cache_load_never_restores_send_state():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "ui" / "main_window.py"
    ).read_text(encoding="utf-8")

    load_start = source.index("def _load_active_wallet_cache")
    load_end = source.index("def ", load_start + 4)
    load_body = source[load_start:load_end]
    assert "self._reset_send_session()" in load_body
