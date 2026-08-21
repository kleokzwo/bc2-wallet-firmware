from pathlib import Path


def _root():
    return Path(__file__).resolve().parents[2]


def test_lockdown_is_explicit_hardware_state():
    h = (_root()/"firmware/firmware/include/bc2_device_state.h").read_text()
    c = (_root()/"firmware/firmware/platform/bc2_device_state.c").read_text()
    assert "BC2_DEVICE_LOCKDOWN" in h
    assert "BC2_DEVICE_EVENT_ENTER_LOCKDOWN" in h
    assert "case BC2_DEVICE_LOCKDOWN" in c


def test_three_pin_failures_are_persistent_lockdown():
    h = (_root()/"firmware/firmware/include/bc2_pin_security.h").read_text()
    c = (_root()/"firmware/firmware/platform/bc2_pin_security.c").read_text()
    assert "BC2_SECURITY_PIN_MAX_FAILURES 3U" in h
    assert "BC2_PIN_SECURITY_LOCKED" in h
    assert "bc2_pin_security_is_locked_down" in c


def test_lockdown_recovery_skips_old_pin_unlock():
    app = (_root()/"firmware/hardware/esp32s3_waveshare/main/app_main.c").read_text()
    assert "recovery->lockdown_recovery" in app
    assert "Lockdown deliberately disables the old PIN" in app
    assert "BC2_PIN_MODE_CREATE" in app
