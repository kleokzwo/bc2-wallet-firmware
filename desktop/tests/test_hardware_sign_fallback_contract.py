from pathlib import Path


def test_hardware_fallback_search_verifies_exact_input_address():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware" / "hardware" / "esp32s3_waveshare"
        / "main" / "bc2_hw_wallet.c"
    ).read_text(encoding="utf-8")

    assert "BC2_WALLET_SIGN_ADDRESS_SEARCH_LIMIT 1024U" in source
    assert "strcmp(derived_address, input_address) == 0" in source
    assert "BC2_HW_SIGN_ERROR_OWNERSHIP" in source


def test_desktop_reports_specific_sign_failure_statuses():
    source = (
        Path(__file__).resolve().parents[1]
        / "bc2" / "services" / "send_service.py"
    ).read_text(encoding="utf-8")

    for status in ("5:", "6:", "7:", "8:"):
        assert status in source
