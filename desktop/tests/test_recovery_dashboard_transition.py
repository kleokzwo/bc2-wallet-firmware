from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]


def test_recovery_in_lockdown_keeps_scanning():
    source=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")
    start=source.index("if d.state == 10:")
    block=source[start:start+1800]
    assert "if self._setup_in_progress:" in block
    assert "QTimer.singleShot(1000, self._device_service.scan)" in block
    assert 'self._set_setup_message("Recovery läuft")' in block


def test_unlocked_scan_finishes_setup_and_enters_dashboard():
    source=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")
    start=source.index("elif d.unlocked:")
    block=source[start:start+3200]
    assert "self._setup_in_progress = False" in block
    assert 'self._navigate("dashboard")' in block


def test_lockdown_recovery_requests_new_pin():
    source=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")
    start=source.index('self._setup_status_title.setText("Recovery-Daten übertragen")')
    block=source[start:start+1400]
    assert "if self._device.state == 10:" in block
    assert "eine neue 4-stellige PIN" in block
