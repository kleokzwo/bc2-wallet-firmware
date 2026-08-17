from __future__ import annotations
import struct
from dataclasses import dataclass
MAGIC=b"BC2"; PROTOCOL_VERSION=1; HEADER_SIZE=9; PAYLOAD_MAX=512; RESPONSE_FLAG=0x80
CMD_PING=0x01; CMD_GET_INFO=0x02; CMD_GET_STATE=0x03; CMD_GET_CAPABILITIES=0x04; CMD_GET_WALLET_STATUS=0x40; CMD_BEGIN_CREATE_WALLET=0x41; CMD_BEGIN_RECEIVE_ADDRESS=0x42; CMD_GET_RECEIVE_RESULT=0x43; CMD_BEGIN_RECOVERY=0x44; CMD_BEGIN_UNLOCK=0x45; CMD_SUBMIT_RECOVERY_MNEMONIC=0x46; CMD_LOCK_WALLET=0x47
class ProtocolError(RuntimeError): pass
@dataclass(frozen=True)
class Frame:
    command:int
    sequence:int
    payload:bytes

def encode_frame(command:int, sequence:int, payload:bytes=b"")->bytes:
    if not 0<=command<=0xff: raise ValueError("command out of range")
    if not 0<=sequence<=0xffff: raise ValueError("sequence out of range")
    if len(payload)>PAYLOAD_MAX: raise ValueError("payload too large")
    return MAGIC+bytes((PROTOCOL_VERSION,command))+struct.pack("<HH",sequence,len(payload))+payload

def decode_frame(data:bytes)->Frame:
    if len(data)<HEADER_SIZE: raise ProtocolError("truncated BC2 frame")
    if data[:3]!=MAGIC: raise ProtocolError("invalid BC2 magic")
    if data[3]!=PROTOCOL_VERSION: raise ProtocolError(f"unsupported protocol version {data[3]}")
    sequence,length=struct.unpack_from("<HH",data,5)
    if length>PAYLOAD_MAX: raise ProtocolError("payload limit exceeded")
    if len(data)!=HEADER_SIZE+length: raise ProtocolError("invalid BC2 frame length")
    return Frame(data[4],sequence,data[HEADER_SIZE:])
