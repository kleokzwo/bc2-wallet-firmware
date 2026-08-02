#!/usr/bin/env python3
"""Probe a BC2 hardware wallet over USB Serial/JTAG.

Requires Python 3 and pyserial (`python -m pip install pyserial`).
No seed, private key or PIN is transmitted.
"""

from __future__ import annotations

import argparse
import struct
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError as exc:
    raise SystemExit("pyserial fehlt. Installation: python -m pip install pyserial") from exc

MAGIC = b"BC2"
VERSION = 1
HEADER_SIZE = 9
CMD_PING = 0x01
CMD_GET_INFO = 0x02
CMD_GET_STATE = 0x03
CMD_GET_CAPABILITIES = 0x04
RESPONSE_FLAG = 0x80


def encode(command: int, sequence: int, payload: bytes = b"") -> bytes:
    if len(payload) > 512:
        raise ValueError("payload too large")
    return MAGIC + bytes((VERSION, command)) + struct.pack("<HH", sequence, len(payload)) + payload


def read_frame(port: serial.Serial, timeout: float) -> tuple[int, int, bytes]:
    deadline = time.monotonic() + timeout
    buffer = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(256)
        if chunk:
            buffer.extend(chunk)
        magic_at = buffer.find(MAGIC)
        if magic_at > 0:
            del buffer[:magic_at]
        if len(buffer) >= HEADER_SIZE:
            payload_size = struct.unpack_from("<H", buffer, 7)[0]
            total_size = HEADER_SIZE + payload_size
            if len(buffer) >= total_size:
                frame = bytes(buffer[:total_size])
                if frame[3] != VERSION:
                    raise RuntimeError(f"unsupported protocol version {frame[3]}")
                sequence, _ = struct.unpack_from("<HH", frame, 5)
                return frame[4], sequence, frame[HEADER_SIZE:]
    raise TimeoutError("no BC2 response received")


def request(port: serial.Serial, command: int, sequence: int, payload: bytes = b"") -> bytes:
    port.reset_input_buffer()
    port.write(encode(command, sequence, payload))
    port.flush()
    response_command, response_sequence, response_payload = read_frame(port, 2.0)
    if response_command != (command | RESPONSE_FLAG):
        raise RuntimeError(f"unexpected response command 0x{response_command:02x}")
    if response_sequence != sequence:
        raise RuntimeError("response sequence mismatch")
    return response_payload


def available_ports() -> list[str]:
    return [entry.device for entry in list_ports.comports()]


def main() -> int:
    parser = argparse.ArgumentParser(description="BC2 hardware USB probe")
    parser.add_argument("--port", help="serial port, for example COM5 or /dev/ttyACM0")
    parser.add_argument("--list", action="store_true", help="list serial ports and exit")
    args = parser.parse_args()

    ports = available_ports()
    if args.list:
        for name in ports:
            print(name)
        return 0
    if not args.port:
        if len(ports) == 1:
            args.port = ports[0]
        else:
            parser.error("--port is required when zero or multiple serial ports exist")

    with serial.Serial(args.port, baudrate=115200, timeout=0.1, write_timeout=2.0) as port:
        ping = request(port, CMD_PING, 1, b"probe")
        info = request(port, CMD_GET_INFO, 2).decode("utf-8", errors="replace")
        state = request(port, CMD_GET_STATE, 3)
        capabilities = request(port, CMD_GET_CAPABILITIES, 4)

    print("BC2-Gerät antwortet.")
    print(f"Ping: {ping.decode('ascii', errors='replace')}")
    print(info)
    if state:
        print(f"Gerätestatus: {state[0]}")
    if len(capabilities) >= 2:
        flags, revision = capabilities[0], capabilities[1]
        names = [name for bit, name in (
            (0x01, "USB"), (0x02, "NVS"), (0x04, "RNG"),
            (0x08, "Display"), (0x10, "Tasten")) if flags & bit]
        print(f"Fähigkeiten: {', '.join(names) if names else 'keine'}")
        print(f"Board-Revision: {revision or 'noch nicht konfiguriert'}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, TimeoutError) as error:
        print(f"Fehler: {error}", file=sys.stderr)
        raise SystemExit(1)
