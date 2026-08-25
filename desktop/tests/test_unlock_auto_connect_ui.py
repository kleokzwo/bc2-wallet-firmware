from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
SOURCE=(ROOT/"desktop/bc2/ui/main_window.py").read_text()
def test_reconnect_hidden_everywhere():
    assert "_setup_scan_button" not in SOURCE
    assert "self._setup_link_separator.setVisible(True)" not in SOURCE
def test_unlock_auto_scans():
    a=SOURCE.index("def _begin_wallet_unlock")
    b=SOURCE.index("def ",a+4)
    block=SOURCE[a:b]
    assert "if self._device is None:" in block
    assert "self._device_service.scan()" in block
