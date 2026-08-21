from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")


def test_logout_has_no_deleted_session_guard_call():
    start = SOURCE.index("def _logout_wallet")
    end = SOURCE.index("def ", start + 4)
    block = SOURCE[start:end]
    assert "_stop_desktop_session_guard" not in block


def test_countdown_feature_has_no_leftover_symbols():
    for token in (
        "_stop_desktop_session_guard",
        "_start_desktop_session_guard",
        "_session_idle_timer",
        "_session_countdown_timer",
        "_session_countdown_dialog",
        "_session_countdown_remaining",
    ):
        assert token not in SOURCE


def test_logout_still_uses_hardware_lock_and_local_cleanup():
    start = SOURCE.index("def _logout_wallet")
    end = SOURCE.index("def ", start + 4)
    block = SOURCE[start:end]
    assert "lock_wallet(self._device.port)" in block
    assert "self._clear_wallet_ui()" in block
    assert "self._wallet_context.deactivate()" in block
    assert 'self._navigate("setup")' in block
