from __future__ import annotations

import hashlib
from functools import lru_cache
from pathlib import Path

VALID_WORD_COUNTS = (12, 24)

@lru_cache(maxsize=1)
def english_words() -> tuple[str, ...]:
    path = Path(__file__).resolve().parent / "assets" / "bip39-english.txt"
    words = tuple(line.strip() for line in path.read_text(encoding="utf-8").splitlines() if line.strip())
    if len(words) != 2048:
        raise RuntimeError("invalid bundled BIP39 word list")
    return words


def normalize_mnemonic(text: str) -> str:
    return " ".join(text.strip().lower().split())


def validate_mnemonic(text: str, expected_words: int | None = None) -> tuple[bool, str, str]:
    mnemonic = normalize_mnemonic(text)
    parts = mnemonic.split() if mnemonic else []
    if expected_words is not None and len(parts) != expected_words:
        return False, f"Es werden genau {expected_words} Wörter erwartet.", mnemonic
    if len(parts) not in VALID_WORD_COUNTS:
        return False, "Recovery unterstützt genau 12 oder 24 BIP39-Wörter.", mnemonic

    words = english_words()
    index = {word: i for i, word in enumerate(words)}
    try:
        indexes = [index[word] for word in parts]
    except KeyError as exc:
        return False, f"Unbekanntes BIP39-Wort: {exc.args[0]}", mnemonic

    total_bits = len(parts) * 11
    checksum_bits = total_bits // 33
    entropy_bits = total_bits - checksum_bits
    bit_string = "".join(f"{i:011b}" for i in indexes)
    entropy = int(bit_string[:entropy_bits], 2).to_bytes(entropy_bits // 8, "big")
    expected = f"{hashlib.sha256(entropy).digest()[0]:08b}"[:checksum_bits]
    if bit_string[entropy_bits:] != expected:
        return False, "Die BIP39-Checksumme ist ungültig. Bitte Wörter und Reihenfolge prüfen.", mnemonic
    return True, "Mnemonic und BIP39-Checksumme sind gültig.", mnemonic


def recovery_fingerprint(normalized_mnemonic: str) -> str:
    digest = hashlib.sha256(normalized_mnemonic.encode("ascii")).hexdigest().upper()
    return f"{digest[:4]} {digest[4:8]}"
