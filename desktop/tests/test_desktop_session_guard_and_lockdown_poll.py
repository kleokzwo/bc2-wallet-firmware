from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
SOURCE=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")


def test_unlock_starts_continuous_setup_state_monitor():
    start=SOURCE.index("def _begin_wallet_unlock")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "self._setup_state_timer.start()" in block
    assert "QTimer.singleShot(250, self._device_service.scan)" in block


def test_setup_state_monitor_polls_without_manual_reconnect():
    start=SOURCE.index("def _poll_setup_device_state")
    end=SOURCE.index("def ", start+4)
    block=SOURCE[start:end]
    assert "self._device_service.scan()" in block
    assert 'self._pages["setup"]' in block
