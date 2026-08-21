from pathlib import Path


def test_recovery_seed_words_use_large_hardware_font():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware"
        / "hardware"
        / "esp32s3_waveshare"
        / "main"
        / "bc2_display_adapter.cpp"
    ).read_text(encoding="utf-8")
    assert 'std::strncmp(title, "RECOVERY ", 9U) == 0' in source
    assert "draw_multiline(body, 75, 2, 3);" in source
