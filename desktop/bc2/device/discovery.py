from __future__ import annotations
from dataclasses import dataclass
import serial
from serial.tools import list_ports
from .client import BC2DeviceClient
from .model import DeviceInfo
@dataclass(frozen=True)
class DiscoveryResult:
    device:DeviceInfo|None
    checked_ports:tuple[str,...]
    errors:tuple[str,...]
class DeviceDiscovery:
    BAUD_RATE=115200
    def available_ports(self): return [p.device for p in list_ports.comports()]
    def find_first(self):
        ports=self.available_ports(); errors=[]
        for name in ports:
            try:
                with serial.Serial(name,baudrate=self.BAUD_RATE,timeout=.1,write_timeout=2.0) as port:
                    return DiscoveryResult(BC2DeviceClient(port).identify(name),tuple(ports),tuple(errors))
            except (OSError,serial.SerialException,RuntimeError,TimeoutError) as exc:
                errors.append(f"{name}: {exc}")
        return DiscoveryResult(None,tuple(ports),tuple(errors))


def begin_create_wallet(port_name: str) -> bool:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE, timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).begin_create_wallet()

def begin_receive_address(port_name: str) -> int:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE,
                       timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).begin_receive_address()


def get_receive_result(port_name: str):
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE,
                       timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).get_receive_result()


def begin_recovery(port_name: str) -> bool:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE, timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).begin_recovery()

def begin_unlock(port_name: str) -> bool:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE, timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).begin_unlock()


def submit_recovery_mnemonic(port_name: str, mnemonic: str) -> bool:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE, timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).submit_recovery_mnemonic(mnemonic)


def lock_wallet(port_name: str) -> bool:
    with serial.Serial(port_name, baudrate=DeviceDiscovery.BAUD_RATE, timeout=.1, write_timeout=2.0) as port:
        return BC2DeviceClient(port).lock_wallet()


def review_transaction(port_name: str, recipient: str,
                       amount: int, change: int, fee: int) -> int:
    with serial.Serial(
        port_name,
        baudrate=DeviceDiscovery.BAUD_RATE,
        timeout=.1,
        write_timeout=2.0,
    ) as port:
        return BC2DeviceClient(port).review_transaction(
            recipient, amount, change, fee
        )


def get_transaction_result(port_name: str) -> int:
    with serial.Serial(
        port_name,
        baudrate=DeviceDiscovery.BAUD_RATE,
        timeout=.1,
        write_timeout=2.0,
    ) as port:
        return BC2DeviceClient(port).get_transaction_result()

def sign_transaction_single(port_name,plan):
    with serial.Serial(port_name,baudrate=DeviceDiscovery.BAUD_RATE,timeout=.1,write_timeout=2.0) as port:return BC2DeviceClient(port).sign_transaction_single(plan)
def get_sign_result(port_name):
    with serial.Serial(port_name,baudrate=DeviceDiscovery.BAUD_RATE,timeout=.1,write_timeout=2.0) as port:return BC2DeviceClient(port).get_sign_result()
