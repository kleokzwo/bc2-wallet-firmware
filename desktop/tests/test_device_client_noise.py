from bc2.device.client import BC2DeviceClient
from bc2.device.protocol import (
    CMD_BEGIN_RECEIVE_ADDRESS, CMD_LOCK_WALLET, RESPONSE_FLAG, encode_frame
)

class FakeSerial:
    def __init__(self, chunks):
        self.chunks = list(chunks)
        self.written = bytearray()

    def read(self, size):
        if self.chunks:
            return self.chunks.pop(0)
        return b""

    def write(self, data):
        self.written.extend(data)
        return len(data)

    def flush(self):
        pass

    def reset_input_buffer(self):
        pass

def test_receive_ignores_console_line_containing_bc2_and_fake_large_length():
    # A real ESP-IDF log line includes the literal text "BC2 frame displayed".
    noise = b'I (1234) bc2_display: BC2 frame displayed through official Waveshare path\r\n'
    # Append bytes that would have caused the old parser to interpret a huge payload.
    noise += b'BC2' + bytes([9, 0x99, 0, 0, 0xff, 0xff]) + b'junk'
    response = encode_frame(
        CMD_BEGIN_RECEIVE_ADDRESS | RESPONSE_FLAG, 1, b'\x01'
    )
    port = FakeSerial([noise[:20], noise[20:] + response])
    client = BC2DeviceClient(port)
    assert client.begin_receive_address() == 1

def test_logout_ignores_console_noise_and_accepts_matching_frame():
    noise = b'I (42) bc2: BC2 application task running\r\n'
    response = encode_frame(CMD_LOCK_WALLET | RESPONSE_FLAG, 1, b'\x01')
    port = FakeSerial([noise, response])
    client = BC2DeviceClient(port)
    assert client.lock_wallet() is True
