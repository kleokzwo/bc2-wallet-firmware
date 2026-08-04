#!/usr/bin/env python3
"""Generate the deterministic, unsigned BC2 hardware-review fixture."""

from pathlib import Path


def compact(value: int) -> bytes:
    if not 0 <= value < 0xFD:
        raise ValueError("fixture only supports one-byte compact integers")
    return bytes((value,))


def main() -> None:
    recipient_script = bytes.fromhex("0014" + "11" * 20)

    unsigned_tx = bytearray()
    unsigned_tx += (2).to_bytes(4, "little")
    unsigned_tx += compact(1)
    unsigned_tx += bytes.fromhex("aa" * 32)
    unsigned_tx += (1).to_bytes(4, "little")
    unsigned_tx += compact(0)
    unsigned_tx += (0xFFFFFFFF).to_bytes(4, "little")
    unsigned_tx += compact(1)
    unsigned_tx += (99_000).to_bytes(8, "little")
    unsigned_tx += compact(len(recipient_script)) + recipient_script
    unsigned_tx += (0).to_bytes(4, "little")

    witness_utxo = (
        (100_000).to_bytes(8, "little")
        + compact(len(recipient_script))
        + recipient_script
    )

    psbt = bytearray(b"psbt\xff")
    psbt += compact(1) + b"\x00" + compact(len(unsigned_tx)) + unsigned_tx + b"\x00"
    psbt += compact(1) + b"\x01" + compact(len(witness_utxo)) + witness_utxo + b"\x00"
    psbt += b"\x00"

    destination = Path(__file__).resolve().parents[1] / "test-data" / "bc2-safe-test-transaction.psbt"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(psbt)
    print(f"Wrote {len(psbt)} bytes to {destination}")


if __name__ == "__main__":
    main()
