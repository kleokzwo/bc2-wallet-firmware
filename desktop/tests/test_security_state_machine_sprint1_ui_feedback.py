from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
def test_policy_and_feedback():
    ui=(ROOT/"desktop/bc2/ui/main_window.py").read_text()
    fw=(ROOT/"firmware/hardware/esp32s3_waveshare/main/app_main.c").read_text()
    assert "if d.state == 10:" in ui
    assert "self._recovery_wallet_button.setEnabled(True)" in ui
    assert "if self._device.wallet_ready or not self._device.setup_required:" in ui
    assert '"PIN FALSCH"' in fw
    assert '"Noch %u Versuche.\\nDanach Recovery."' in fw
