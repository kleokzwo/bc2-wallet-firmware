#!/usr/bin/env python3
"""BC2 Cold Wallet USB protocol probe - human readable.

Read-only diagnostic requests only:
  0x01 PING
  0x03 GET_STATE
  0x40 GET_WALLET_STATUS

No PIN, mnemonic, seed, private key, transaction, or signing request is sent.
Requires: python -m pip install pyserial
"""
from __future__ import annotations

import argparse
import struct
import sys
import time
from dataclasses import dataclass

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:
    raise SystemExit("pyserial fehlt. Installation: python -m pip install pyserial") from exc

PROBE_VERSION = "0.51.5"
MAGIC = b"BC2"
VERSION = 1
HEADER_SIZE = 9
RESPONSE_FLAG = 0x80

COMMANDS = {
    0x01: "PING",
    0x02: "GET_INFO",
    0x03: "GET_STATE",
    0x04: "GET_CAPABILITIES",
    0x10: "DISPLAY_TEST",
    0x11: "BUTTON_TEST",
    0x20: "REVIEW_RECEIVE_ADDRESS",
    0x21: "REVIEW_TRANSACTION",
    0x22: "GET_TRANSACTION_RESULT",
    0x23: "SIGN_TRANSACTION",
    0x24: "GET_SIGN_RESULT",
    0x40: "GET_WALLET_STATUS",
    0x41: "BEGIN_CREATE_WALLET",
    0x42: "BEGIN_RECEIVE_ADDRESS",
    0x43: "GET_RECEIVE_RESULT",
    0x44: "BEGIN_RECOVERY",
    0x45: "BEGIN_UNLOCK",
    0x46: "SUBMIT_RECOVERY_MNEMONIC",
    0x47: "LOCK_WALLET",
    0x48: "GET_WALLET_ID",
}

STATES = {
    0: "BOOT",
    1: "SETUP_REQUIRED - keine Wallet installiert",
    2: "LOCKED - Wallet installiert, aber gesperrt",
    3: "UNLOCKING - Entsperrvorgang läuft",
    4: "COOLDOWN - Wartephase nach Fehlversuchen",
    5: "DASHBOARD - Wallet ist entsperrt",
    6: "RECEIVE_REVIEW - Empfang wird am Gerät geprüft",
    7: "TRANSACTION_REVIEW - Transaktion wird am Gerät geprüft",
    8: "SETTINGS - Geräteeinstellungen",
    9: "ERROR - Fehlerzustand",
    10: "LOCKDOWN - nur Recovery erlaubt",
}

WALLET_STATUS = {
    0: "KEINE WALLET installiert",
    2: "WALLET installiert",
}


@dataclass
class Frame:
    command: int
    sequence: int
    payload: bytes
    raw: bytes
    garbage_before: bytes


def hexline(data: bytes) -> str:
    return " ".join(f"{b:02x}" for b in data)


def printable(data: bytes) -> str:
    return "".join(chr(b) if 32 <= b <= 126 else "." for b in data)


def encode(command: int, sequence: int, payload: bytes = b"") -> bytes:
    if len(payload) > 512:
        raise ValueError("Payload größer als 512 Byte")
    return MAGIC + bytes((VERSION, command)) + struct.pack("<HH", sequence, len(payload)) + payload


def read_available(port: serial.Serial, duration: float) -> bytes:
    deadline = time.monotonic() + duration
    out = bytearray()
    while time.monotonic() < deadline:
        waiting = port.in_waiting
        chunk = port.read(waiting if waiting else 1)
        if chunk:
            out.extend(chunk)
    return bytes(out)


def read_frame(port: serial.Serial, timeout: float = 3.0) -> Frame:
    deadline = time.monotonic() + timeout
    buffer = bytearray()
    garbage = bytearray()

    while time.monotonic() < deadline:
        chunk = port.read(256)
        if chunk:
            buffer.extend(chunk)

        magic_at = buffer.find(MAGIC)
        if magic_at > 0:
            garbage.extend(buffer[:magic_at])
            del buffer[:magic_at]
        elif magic_at < 0 and len(buffer) > 2:
            # Keep 2 bytes because they may be the beginning of the 3-byte magic "BC2".
            garbage.extend(buffer[:-2])
            del buffer[:-2]

        if len(buffer) >= HEADER_SIZE and buffer[:3] == MAGIC:
            payload_len = struct.unpack_from("<H", buffer, 7)[0]
            if payload_len > 512:
                raise RuntimeError(f"Ungültige Payload-Länge im Frame: {payload_len}")
            total = HEADER_SIZE + payload_len
            if len(buffer) >= total:
                raw = bytes(buffer[:total])
                if raw[3] != VERSION:
                    raise RuntimeError(f"Nicht unterstützte Protokollversion: {raw[3]}")
                sequence = struct.unpack_from("<H", raw, 5)[0]
                return Frame(raw[4], sequence, raw[HEADER_SIZE:], raw, bytes(garbage))

    raise TimeoutError("Keine vollständige BC2-Antwort innerhalb des Timeouts empfangen")


def explain_header(raw: bytes, command: int, sequence: int, payload: bytes) -> None:
    base = command & 0x7F
    name = COMMANDS.get(base, "UNKNOWN")
    is_response = bool(command & RESPONSE_FLAG)
    print("  Frame-Aufbau:")
    print("    42 43 32       -> Magic = ASCII 'BC2'; markiert den Anfang eines BC2-Frames")
    print(f"    {raw[3]:02x}             -> Version = {raw[3]}; aktuell verwendete Protokollversion")
    if is_response:
        print(f"    {command:02x}             -> Command = 0x{command:02X}; Antwort auf {name} (0x{base:02X} + Response-Bit 0x80)")
    else:
        print(f"    {command:02x}             -> Command = 0x{command:02X}; {name}")
    print(f"    {raw[5]:02x} {raw[6]:02x}          -> Sequence = {sequence}; ordnet Antwort und Anfrage einander zu")
    print(f"    {raw[7]:02x} {raw[8]:02x}          -> Payload Length = {len(payload)} Byte(s)")
    print(f"    Payload        -> {hexline(payload) if payload else '(leer)'}")


def explain_payload(base_command: int, payload: bytes) -> None:
    print("  Bedeutung der Antwort:")
    if base_command == 0x01:
        text = payload.decode("ascii", errors="replace")
        print(f"    PING-Echo = {text!r}")
        print("    -> Beweist: USB-Transport, BC2-Parser und Response-Pfad arbeiten zusammen.")
    elif base_command == 0x03:
        if not payload:
            print("    FEHLER: GET_STATE enthält kein Statusbyte.")
        else:
            value = payload[0]
            print(f"    State = {value}: {STATES.get(value, 'UNBEKANNT')}")
            print("    -> Das ist der aktuelle Zustand des Hardware-Wallet-State-Machines.")
    elif base_command == 0x40:
        if not payload:
            print("    FEHLER: GET_WALLET_STATUS enthält kein Statusbyte.")
        else:
            value = payload[0]
            print(f"    Wallet Status = {value}: {WALLET_STATUS.get(value, 'UNBEKANNT')}")
            print("    -> Prüft nur, ob auf dem Gerät eine Wallet initialisiert ist.")
    else:
        print(f"    Rohdaten = {hexline(payload) if payload else '(leer)'}")


def show_foreign_bytes(title: str, data: bytes) -> None:
    print(f"  WARNUNG: {title}: {len(data)} fremde Byte(s)")
    print(f"    HEX   : {hexline(data)}")
    print(f"    ASCII : {printable(data)}")
    print("    -> Diese Bytes gehören nicht zu einem BC2-Frame und dürfen auf dem BC2-Protokollkanal nicht erscheinen.")


def print_frame(direction: str, command: int, sequence: int, raw: bytes, payload: bytes) -> None:
    base = command & 0x7F
    name = COMMANDS.get(base, "UNKNOWN")
    kind = "ANTWORT" if command & RESPONSE_FLAG else "ANFRAGE"
    print(f"{direction}: {kind} {name}")
    print(f"  RAW HEX: {hexline(raw)}")
    explain_header(raw, command, sequence, payload)


def run_request(port: serial.Serial, command: int, sequence: int, payload: bytes = b"") -> bool:
    request = encode(command, sequence, payload)
    print("=" * 84)
    print_frame("TX", command, sequence, request, payload)

    # Only clear bytes already present. New foreign bytes emitted while processing the
    # request are kept and reported by read_frame().
    port.reset_input_buffer()
    started = time.monotonic()
    port.write(request)
    port.flush()
    response = read_frame(port)
    elapsed_ms = (time.monotonic() - started) * 1000.0

    print(f"  Antwortzeit: {elapsed_ms:.1f} ms")
    if elapsed_ms > 500.0:
        print("  HINWEIS: Antwort ist korrekt, aber fuer einen Diagnose-Request ungewoehnlich langsam.")
    print()
    channel_clean = not response.garbage_before
    if response.garbage_before:
        show_foreign_bytes("USB-Kanal vor der BC2-Antwort NICHT sauber", response.garbage_before)
    else:
        print("  OK: Vor der Antwort kamen keine fremden Bytes.")

    print()
    print_frame("RX", response.command, response.sequence, response.raw, response.payload)

    expected = command | RESPONSE_FLAG
    logical_ok = True
    if response.command != expected:
        print(f"  FEHLER: erwartet 0x{expected:02X}, erhalten 0x{response.command:02X}")
        logical_ok = False
    if response.sequence != sequence:
        print(f"  FEHLER: Sequence erwartet {sequence}, erhalten {response.sequence}")
        logical_ok = False

    explain_payload(command, response.payload)
    if logical_ok:
        print("  OK: Command und Sequence der Antwort sind korrekt.")
    print()
    return logical_ok and channel_clean


def choose_port(explicit: str | None) -> str:
    if explicit:
        return explicit
    ports = [p.device for p in list_ports.comports()]
    if len(ports) == 1:
        return ports[0]
    if not ports:
        raise RuntimeError("Kein serielles USB-Gerät gefunden. Beispiel: --port /dev/ttyACM0")
    raise RuntimeError("Mehrere serielle Geräte gefunden: " + ", ".join(ports) + ". Bitte --port angeben.")


def main() -> int:
    parser = argparse.ArgumentParser(description="BC2 USB-Protokoll lesbar diagnostizieren")
    parser.add_argument("--port", help="z.B. /dev/ttyACM0 oder COM5")
    parser.add_argument("--list", action="store_true", help="serielle Ports auflisten")
    parser.add_argument("--startup-wait", type=float, default=1.2,
                        help="Sekunden nach Port-Öffnung auf Boot-Ausgaben warten (Standard: 1.2)")
    args = parser.parse_args()

    if args.list:
        for p in list_ports.comports():
            print(f"{p.device}: {p.description}")
        return 0

    port_name = choose_port(args.port)
    print(f"BC2 Cold Wallet - USB Protocol Probe v{PROBE_VERSION}")
    print(f"Port: {port_name}")
    print("Sicherer Diagnosetest: nur PING, GET_STATE und GET_WALLET_STATUS.")
    print("Es werden KEINE PINs, Seeds, Mnemonics, Private Keys oder Signaturen gesendet.\n")

    all_clean = True
    with serial.Serial(port_name, baudrate=115200, timeout=0.05, write_timeout=2.0) as port:
        # Opening a native ESP32-S3 serial endpoint may reset the board. Capture anything
        # emitted during that startup window instead of silently mixing it into request #1.
        startup = read_available(port, max(0.0, args.startup_wait))
        if startup:
            print("=" * 84)
            show_foreign_bytes("STARTUP-Ausgabe auf dem BC2-USB-Kanal erkannt", startup)
            print()
            all_clean = False
        else:
            print("STARTUP: OK - keine Klartext-/Boot-Ausgabe auf dem BC2-Protokollkanal.\n")

        all_clean &= run_request(port, 0x01, 1, b"bc2-probe-test")
        all_clean &= run_request(port, 0x03, 2)
        all_clean &= run_request(port, 0x40, 3)

    print("=" * 84)
    if all_clean:
        print("GESAMTERGEBNIS: PASS")
        print("BC2 antwortet korrekt und der USB-Protokollkanal ist frei von fremden Bytes.")
        return 0

    print("GESAMTERGEBNIS: FAIL")
    print("BC2-Antworten wurden empfangen, aber mindestens ein Kanal-/Frame-Problem wurde erkannt.")
    return 2


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, TimeoutError, ValueError, serial.SerialException) as exc:
        print(f"FEHLER: {exc}", file=sys.stderr)
        raise SystemExit(1)
