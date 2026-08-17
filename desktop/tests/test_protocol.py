import unittest
from bc2.device.protocol import *
class T(unittest.TestCase):
    def test_roundtrip(self):
        f=decode_frame(encode_frame(2,42,b"hello")); self.assertEqual((f.command,f.sequence,f.payload),(2,42,b"hello"))
    def test_magic(self): self.assertEqual(encode_frame(1,1)[:3],b"BC2")
    def test_limit(self):
        with self.assertRaises(ValueError): encode_frame(1,1,b"x"*513)
    def test_bad_magic(self):
        x=bytearray(encode_frame(1,1)); x[:3]=b"BAD"
        with self.assertRaises(ProtocolError): decode_frame(bytes(x))
if __name__=="__main__": unittest.main()
