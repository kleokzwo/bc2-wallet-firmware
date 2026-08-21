from pathlib import Path


def test_sign_request_carries_derivation_index_hint():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "device" / "client.py"
    ).read_text(encoding="utf-8")
    assert "derivation_index.to_bytes(4, \"little\")" in source
    assert "bytes((3, plan.input_count, position))" in source


def test_hardware_verifies_hint_against_derived_address():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware" / "hardware" / "esp32s3_waveshare"
        / "main" / "bc2_hw_wallet.c"
    ).read_text(encoding="utf-8")
    assert "input_index" in source
    assert "strcmp(derived_address, input_address) != 0" in source
    assert "goto cleanup;" in source
