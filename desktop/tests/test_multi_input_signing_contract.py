from pathlib import Path


def _desktop(relative: str) -> str:
    return (
        Path(__file__).resolve().parents[1] / relative
    ).read_text(encoding="utf-8")


def test_one_input_sprint_guard_is_gone():
    source = _desktop("bc2/services/send_service.py")
    assert "Dieser Sprint signiert bewusst nur" not in source
    assert "for position in range(self.plan.input_count)" in source
    assert "_build_signed(self.plan, signed_inputs)" in source


def test_sign_protocol_is_multi_input_v3():
    source = _desktop("bc2/device/client.py")
    assert "bytes((3, plan.input_count, position))" in source
    assert "hash_prevouts" in source
    assert "hash_sequence" in source


def test_firmware_tracks_whole_signing_session():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware/firmware/platform/bc2_device_service.c"
    ).read_text(encoding="utf-8")
    assert "sign_session_total_amount" in source
    assert "new_total != expected_total" in source
    assert "sign_session_next_position" in source


def test_hardware_uses_multi_input_bip143_and_owns_change():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware/hardware/esp32s3_waveshare/main/bc2_hw_wallet.c"
    ).read_text(encoding="utf-8")
    assert "bc2_p2wpkh_sighash_all_multi" in source
    assert "change_owned" in source
    assert "strcmp(change_candidate, change_address) == 0" in source
