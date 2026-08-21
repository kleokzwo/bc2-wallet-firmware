from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
SOURCE=(ROOT/"desktop/bc2/ui/main_window.py").read_text(encoding="utf-8")


def test_desktop_countdown_is_completely_removed():
    for token in (
        "_session_idle_timer",
        "_session_countdown_timer",
        "_session_countdown_remaining",
        "_session_countdown_dialog",
        "_start_session_countdown",
        "_tick_session_countdown",
        "_start_desktop_session_guard",
        "def eventFilter",
    ):
        assert token not in SOURCE


def test_failed_lock_message_is_clear():
    assert '"Bitte entsperre zuerst die Hardware Wallet."' in SOURCE
    assert '"Die Hardware Wallet konnte nicht gesperrt werden."' not in SOURCE
