from pathlib import Path


def test_recovery_replace_requires_new_pin_before_wallet_reset():
    source = (
        Path(__file__).resolve().parents[2]
        / "firmware"
        / "hardware"
        / "esp32s3_waveshare"
        / "main"
        / "app_main.c"
    ).read_text(encoding="utf-8")

    old_pin_branch = source.index(
        "pin_session->post_action == BC2_PIN_POST_RECOVERY"
    )
    new_pin_mode = source.index(
        "pin_session->mode = BC2_PIN_MODE_CREATE",
        old_pin_branch,
    )
    replacement_commit = source.index(
        "recovery->replacing_existing &&",
        new_pin_mode,
    )
    reset = source.index("bc2_hw_wallet_factory_reset(hal)", replacement_commit)

    assert old_pin_branch < new_pin_mode < replacement_commit < reset
