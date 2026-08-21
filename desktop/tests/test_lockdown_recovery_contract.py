from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]

def test_lockdown_ui_is_minimal():
    source=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")
    start=source.index("if d.state == 10:")
    block=source[start:start+1800]
    assert 'self._set_setup_message("Recovery erforderlich")' in block
    assert 'self._set_setup_message("Recovery läuft")' in block
    assert "Nach 3 falschen PIN-Versuchen" not in block

def test_begin_recovery_accepts_lockdown_not_normal_locked():
    source=(ROOT/"firmware/firmware/platform/bc2_device_service.c").read_text(encoding="utf-8")
    block=source[source.index("case BC2_USB_CMD_BEGIN_RECOVERY"):source.index("case BC2_USB_CMD_SUBMIT_RECOVERY_MNEMONIC")]
    assert "BC2_DEVICE_LOCKDOWN" in block
    assert "BC2_DEVICE_LOCKED" not in block

def test_submit_recovery_accepts_lockdown_not_normal_locked():
    source=(ROOT/"firmware/firmware/platform/bc2_device_service.c").read_text(encoding="utf-8")
    block=source[source.index("case BC2_USB_CMD_SUBMIT_RECOVERY_MNEMONIC"):source.index("case BC2_USB_CMD_BEGIN_UNLOCK")]
    assert "BC2_DEVICE_LOCKDOWN" in block
    assert "BC2_DEVICE_LOCKED" not in block

def test_hardware_lockdown_text_is_minimal():
    source=(ROOT/"firmware/hardware/esp32s3_waveshare/main/app_main.c").read_text(encoding="utf-8")
    block=source[source.index("machine->state == BC2_DEVICE_LOCKDOWN"):source.index("} else {",source.index("machine->state == BC2_DEVICE_LOCKDOWN"))]
    assert 'view.secondary_text = "";' in block
