import unittest
from bc2.device.client import BC2DeviceClient
from bc2.device.protocol import *
class Fake:
    def __init__(self): self.r=b""
    def reset_input_buffer(self): self.r=b""
    def write(self,data):
        q=decode_frame(data)
        address=b"bc1qexampleapprovedaddress000000000000000000"
        payload={
            CMD_PING:b"pong",
            CMD_GET_INFO:b"BC2 Cold Wallet 0.35.0",
            CMD_GET_STATE:bytes([5]),
            CMD_GET_CAPABILITIES:bytes([0x1f,2]),
            CMD_GET_WALLET_STATUS:bytes([2]),
            CMD_BEGIN_CREATE_WALLET:bytes([1]),
            CMD_BEGIN_RECEIVE_ADDRESS:bytes([1]),
            CMD_GET_RECEIVE_RESULT:bytes([1,len(address)])+address,
        }[q.command]
        self.r=encode_frame(q.command|RESPONSE_FLAG,q.sequence,payload)
        return len(data)
    def flush(self): pass
    def read(self,size=1): x=self.r[:size]; self.r=self.r[size:]; return x
class T(unittest.TestCase):
    def test_identify(self):
        d=BC2DeviceClient(Fake()).identify('/dev/ttyACM0'); self.assertEqual(d.state,5); self.assertEqual(d.board_revision,2); self.assertEqual(d.capabilities,("USB","NVS","RNG","Display","Tasten")); self.assertTrue(d.wallet_ready)

    def test_receive_result_is_only_returned_after_approval(self):
        client=BC2DeviceClient(Fake())
        self.assertEqual(client.begin_receive_address(),1)
        status,address=client.get_receive_result()
        self.assertEqual(status,1)
        self.assertTrue(address.startswith("bc1"))
if __name__=="__main__": unittest.main()
