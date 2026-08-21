from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
SOURCE=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")


def test_idle_timeout_is_15_minutes():
    assert "15 * 60 * 1000" in SOURCE


def test_inactivity_opens_60_second_modal_popup():
    start=SOURCE.index("def _start_session_countdown")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "self._session_countdown_remaining = 60" in block
    assert "QMessageBox(self)" in block
    assert "Qt.ApplicationModal" in block
    assert "Klicke OK, um weiterzuarbeiten." in block
    assert "dialog.show()" in block


def test_popup_ok_resets_inactivity_timer():
    start=SOURCE.index("def _acknowledge_session_countdown")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "self._session_countdown_timer.stop()" in block
    assert "self._session_countdown_remaining = 60" in block
    assert "self._session_idle_timer.start()" in block


def test_activity_does_not_cancel_visible_popup():
    start=SOURCE.index("def _register_user_activity")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "if self._session_countdown_dialog is not None:" in block
    assert "return" in block


def test_countdown_zero_uses_normal_logout_path():
    start=SOURCE.index("def _tick_session_countdown")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "self._logout_wallet()" in block


def test_old_banner_was_removed():
    assert "_session_warning" not in SOURCE
