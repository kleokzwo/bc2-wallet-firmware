from pathlib import Path


def _app_source() -> str:
    return (
        Path(__file__).resolve().parents[2]
        / "firmware"
        / "hardware"
        / "esp32s3_waveshare"
        / "main"
        / "app_main.c"
    ).read_text(encoding="utf-8")


def test_physical_unlock_never_inherits_old_pin_mode():
    source = _app_source()
    marker = "if (machine->state == BC2_DEVICE_UNLOCKING)"
    start = source.index(marker)
    body = source[start:start + 1600]
    assert "pin_session->mode = BC2_PIN_MODE_UNLOCK;" in body
    assert "pin_session->post_action = BC2_PIN_POST_NONE;" in body


def test_session_timeout_clears_transient_authorization_state():
    source = _app_source()
    assert "Session timeout is a security boundary" in source
    assert "pin_session.active = false;" in source
    assert "bc2_device_service_clear_wallet_id(&device_service);" in source
