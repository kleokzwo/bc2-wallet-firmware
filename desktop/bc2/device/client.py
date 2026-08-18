from __future__ import annotations
import time
from .model import DeviceInfo
from .protocol import *
class BC2DeviceClient:
    def __init__(self,serial_port): self._port=serial_port; self._sequence=0
    def _next_sequence(self):
        self._sequence=(self._sequence+1)&0xffff
        if self._sequence==0: self._sequence=1
        return self._sequence
    def _read_frame(self, expected_command: int, expected_sequence: int, timeout=5.0):
        """Read one BC2 response from a noisy ESP32 USB Serial/JTAG stream.

        ESP-IDF console logs and the BC2 binary protocol currently share the same
        physical serial stream. Log text may itself contain the characters
        ``BC2``. Therefore a bare MAGIC search is not sufficient: every candidate
        must also match protocol version, response command, sequence and payload
        bounds before it is accepted.
        """
        deadline = None if timeout is None else time.monotonic() + timeout
        buf = bytearray()
        expected_response = expected_command | RESPONSE_FLAG

        while deadline is None or time.monotonic() < deadline:
            chunk = self._port.read(256)
            if chunk:
                buf.extend(chunk)

            while True:
                pos = buf.find(MAGIC)
                if pos < 0:
                    if len(buf) > len(MAGIC) - 1:
                        del buf[:-(len(MAGIC) - 1)]
                    break

                if pos > 0:
                    del buf[:pos]

                if len(buf) < HEADER_SIZE:
                    break

                # A console line containing "BC2" is not a protocol frame.
                if buf[3] != PROTOCOL_VERSION:
                    del buf[0]
                    continue

                command = buf[4]
                sequence = int.from_bytes(buf[5:7], "little")
                length = int.from_bytes(buf[7:9], "little")

                if command != expected_response or sequence != expected_sequence:
                    del buf[0]
                    continue

                # Do not abort on a false MAGIC candidate from console output.
                if length > PAYLOAD_MAX:
                    del buf[0]
                    continue

                total = HEADER_SIZE + length
                if len(buf) < total:
                    break

                return bytes(buf[:total])

        raise TimeoutError("no BC2 response received")

    def request(self,command,payload=b"",timeout=5.0):
        seq=self._next_sequence()
        self._port.reset_input_buffer()
        self._port.write(encode_frame(command,seq,payload))
        self._port.flush()
        r=decode_frame(self._read_frame(command, seq, timeout))
        return r.payload
    def identify(self,port_name):
        if not self.request(CMD_PING,b"desktop-v0.41.0"): raise ProtocolError("empty ping response")
        info=self.request(CMD_GET_INFO).decode("utf-8",errors="replace").strip()
        if not info or "BC2" not in info.upper(): raise ProtocolError("device does not identify as BC2")
        state=self.request(CMD_GET_STATE); caps=self.request(CMD_GET_CAPABILITIES)
        wallet=self.request(CMD_GET_WALLET_STATUS)
        return DeviceInfo(port_name,info,state[0] if state else None,caps[0] if len(caps)>0 else 0,caps[1] if len(caps)>1 else 0,wallet[0] if wallet else 0)
    def begin_create_wallet(self):
        payload=self.request(CMD_BEGIN_CREATE_WALLET)
        if len(payload)!=1: raise ProtocolError("invalid create-wallet response")
        return payload[0] == 1

    def begin_recovery(self):
        payload = self.request(CMD_BEGIN_RECOVERY)
        if len(payload) != 1:
            raise ProtocolError("invalid recovery response")
        return payload[0] == 1

    def begin_unlock(self):
        payload = self.request(CMD_BEGIN_UNLOCK)
        if len(payload) != 1:
            raise ProtocolError("invalid unlock response")
        return payload[0] == 1

    def lock_wallet(self):
        payload = self.request(CMD_LOCK_WALLET)
        if len(payload) != 1:
            raise ProtocolError("invalid lock-wallet response")
        return payload[0] == 1

    def submit_recovery_mnemonic(self, mnemonic: str):
        raw = mnemonic.encode("ascii", errors="strict")
        payload = self.request(CMD_SUBMIT_RECOVERY_MNEMONIC, raw, timeout=3.0)
        if len(payload) != 1:
            raise ProtocolError("invalid recovery-mnemonic response")
        return payload[0] == 1

    def begin_receive_address(self):
        payload = self.request(CMD_BEGIN_RECEIVE_ADDRESS)
        if len(payload) != 1:
            raise ProtocolError("invalid begin-receive response")
        return payload[0]

    def get_receive_result(self):
        payload = self.request(CMD_GET_RECEIVE_RESULT)
        if len(payload) < 1:
            raise ProtocolError("invalid receive-result response")
        status = payload[0]
        if status != 1:
            return status, None
        if len(payload) < 2:
            raise ProtocolError("approved receive result has no address length")
        length = payload[1]
        if length == 0 or len(payload) != length + 2:
            raise ProtocolError("invalid approved receive address")
        address = payload[2:].decode("ascii", errors="strict")
        return status, address



    def review_transaction(self, recipient: str, amount: int,
                           change: int, fee: int):
        raw_address = recipient.encode("ascii", errors="strict")
        if not raw_address or len(raw_address) > 95:
            raise ValueError("invalid transaction recipient address length")
        if amount <= 0 or change < 0 or fee < 0:
            raise ValueError("invalid transaction amounts")

        payload = (
            bytes((1, 1, 1))
            + int(amount).to_bytes(8, "little", signed=False)
            + int(change).to_bytes(8, "little", signed=False)
            + int(fee).to_bytes(8, "little", signed=False)
            + bytes((len(raw_address),))
            + raw_address
        )
        response = self.request(
            CMD_REVIEW_TRANSACTION,
            payload,
            timeout=None,
        )
        if len(response) != 1:
            raise ProtocolError("invalid transaction-review response")
        return response[0]

    def get_transaction_result(self):
        response = self.request(
            CMD_GET_TRANSACTION_RESULT,
            timeout=None,
        )
        if len(response) != 1:
            raise ProtocolError("invalid transaction-result response")
        return response[0]
