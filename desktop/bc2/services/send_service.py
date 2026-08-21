from __future__ import annotations

from dataclasses import dataclass
from decimal import Decimal, InvalidOperation

from PySide6.QtCore import QObject, QThread, Signal, Slot

from .electrum_service import (
    fetch_utxos,
    address_to_scriptpubkey,
    broadcast_transaction,
)
from bc2.device.discovery import review_transaction, get_transaction_result, sign_transaction_input, get_sign_result
import time

SATOSHIS_PER_BC2 = 100_000_000
DEFAULT_FEE_RATE = 2
DUST_LIMIT = 546


@dataclass(frozen=True)
class SignedTransaction:
    raw_hex:str
    txid:str
    wtxid:str
    public_key_hex:str
    signature_hex:str

@dataclass(frozen=True)
class SendPlan:
    recipient: str
    amount: int
    fee: int
    change: int
    selected_total: int
    input_count: int
    estimated_vbytes: int
    fee_rate: int
    utxos: tuple


def bc2_to_satoshis(value) -> int:
    try:
        # Accept str as intended, but also tolerate an older SendPage passing float.
        normalized = str(value).strip().replace(",", ".")
        amount = Decimal(normalized)
    except (InvalidOperation, ValueError, TypeError) as exc:
        raise ValueError("Bitte einen gültigen Betrag eingeben.") from exc
    if amount <= 0:
        raise ValueError("Der Betrag muss größer als 0 sein.")
    satoshis = amount * SATOSHIS_PER_BC2
    if satoshis != satoshis.to_integral_value():
        raise ValueError("Maximal 8 Nachkommastellen sind erlaubt.")
    return int(satoshis)


def _estimate_vbytes(inputs: int, with_change: bool) -> int:
    outputs = 2 if with_change else 1
    return 10 + inputs * 68 + outputs * 31


def create_send_plan(utxos, recipient: str, amount: int,
                     fee_rate: int = DEFAULT_FEE_RATE) -> SendPlan:
    selected = []
    total = 0
    for utxo in sorted(utxos, key=lambda item: item.value, reverse=True):
        selected.append(utxo)
        total += utxo.value

        with_change_vbytes = _estimate_vbytes(len(selected), True)
        fee = fee_rate * with_change_vbytes
        if total < amount + fee:
            continue

        change = total - amount - fee
        if change >= DUST_LIMIT:
            return SendPlan(
                recipient, amount, fee, change, total, len(selected),
                with_change_vbytes, fee_rate, tuple(selected)
            )

        no_change_vbytes = _estimate_vbytes(len(selected), False)
        minimum_fee = fee_rate * no_change_vbytes
        if total >= amount + minimum_fee:
            return SendPlan(
                recipient, amount, total - amount, 0, total, len(selected),
                no_change_vbytes, fee_rate, tuple(selected)
            )

    raise ValueError(
        "Nicht genügend verfügbares Guthaben für Betrag und Netzwerkgebühr."
    )



def _compact_size(value: int) -> bytes:
    if value < 0xFD:
        return bytes((value,))
    if value <= 0xFFFF:
        return b"\xfd" + value.to_bytes(2, "little")
    if value <= 0xFFFFFFFF:
        return b"\xfe" + value.to_bytes(4, "little")
    return b"\xff" + value.to_bytes(8, "little")


def _sha256d(data: bytes) -> bytes:
    import hashlib
    return hashlib.sha256(hashlib.sha256(data).digest()).digest()


def _change_address(plan) -> str:
    if not plan.utxos:
        raise ValueError("Transaktion enthält keine Inputs.")
    return plan.utxos[0].address


def _serialize_outputs(plan) -> bytes:
    recipient_script = address_to_scriptpubkey(plan.recipient)
    result = bytearray()
    result += _compact_size(2 if plan.change else 1)
    result += int(plan.amount).to_bytes(8, "little")
    result += _compact_size(len(recipient_script)) + recipient_script
    if plan.change:
        change_script = address_to_scriptpubkey(_change_address(plan))
        result += int(plan.change).to_bytes(8, "little")
        result += _compact_size(len(change_script)) + change_script
    return bytes(result)


def _serialize_unsigned(plan) -> bytes:
    result = bytearray((2).to_bytes(4, "little"))
    result += _compact_size(len(plan.utxos))
    for utxo in plan.utxos:
        result += bytes.fromhex(utxo.tx_hash)[::-1]
        result += int(utxo.tx_pos).to_bytes(4, "little")
        result += b"\x00"
        result += (0xFFFFFFFD).to_bytes(4, "little")
    result += _serialize_outputs(plan)
    result += (0).to_bytes(4, "little")
    return bytes(result)


def _sign_context(plan) -> tuple[bytes, bytes]:
    prevouts = bytearray()
    sequences = bytearray()
    for utxo in plan.utxos:
        prevouts += bytes.fromhex(utxo.tx_hash)[::-1]
        prevouts += int(utxo.tx_pos).to_bytes(4, "little")
        sequences += (0xFFFFFFFD).to_bytes(4, "little")
    return _sha256d(bytes(prevouts)), _sha256d(bytes(sequences))


def _build_signed(plan, signed_inputs) -> SignedTransaction:
    if len(signed_inputs) != len(plan.utxos):
        raise ValueError("Nicht alle Inputs wurden signiert.")

    result = bytearray((2).to_bytes(4, "little"))
    result += b"\x00\x01"
    result += _compact_size(len(plan.utxos))
    for utxo in plan.utxos:
        result += bytes.fromhex(utxo.tx_hash)[::-1]
        result += int(utxo.tx_pos).to_bytes(4, "little")
        result += b"\x00"
        result += (0xFFFFFFFD).to_bytes(4, "little")
    result += _serialize_outputs(plan)

    pubkeys = []
    signatures_hex = []
    for public_key, signature in signed_inputs:
        sig_with_type = signature + b"\x01"
        result += b"\x02"
        result += _compact_size(len(sig_with_type)) + sig_with_type
        result += _compact_size(len(public_key)) + public_key
        pubkeys.append(public_key.hex())
        signatures_hex.append(signature.hex())

    result += (0).to_bytes(4, "little")
    txid = _sha256d(_serialize_unsigned(plan))[::-1].hex()
    wtxid = _sha256d(bytes(result))[::-1].hex()

    return SignedTransaction(
        bytes(result).hex(),
        txid,
        wtxid,
        ",".join(pubkeys),
        ",".join(signatures_hex),
    )

class _Worker(QObject):
    finished = Signal(object)
    failed = Signal(str)

    def __init__(self, server, addresses, recipient, amount):
        super().__init__()
        self.server = server
        self.addresses = list(addresses)
        self.recipient = recipient
        self.amount = amount

    @Slot()
    def run(self):
        try:
            utxos = fetch_utxos(self.server, self.addresses)
            self.finished.emit(
                create_send_plan(utxos, self.recipient, self.amount)
            )
        except Exception as exc:
            self.failed.emit(str(exc))


class SendService(QObject):
    started = Signal()
    finished = Signal(object)
    failed = Signal(str)

    review_started = Signal()
    review_progress = Signal(str)
    review_finished = Signal(bool)
    review_failed = Signal(str)
    sign_started=Signal()
    sign_progress=Signal(str)
    sign_finished=Signal(object)
    sign_failed=Signal(str)

    broadcast_started = Signal()
    broadcast_finished = Signal(str)
    broadcast_failed = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._thread = None
        self._worker = None
        self._review_thread = None
        self._review_worker = None
        self._sign_thread=None
        self._sign_worker=None
        self._broadcast_thread = None
        self._broadcast_worker = None

    def prepare(self, server, addresses, recipient, amount_text):
        if self._thread is not None:
            return
        try:
            amount = bc2_to_satoshis(amount_text)
        except Exception as exc:
            self.failed.emit(str(exc))
            return

        self.started.emit()
        thread = QThread(self)
        worker = _Worker(server, addresses, recipient, amount)
        worker.moveToThread(thread)
        thread.started.connect(worker.run)
        worker.finished.connect(self._done)
        worker.failed.connect(self._fail)
        worker.finished.connect(thread.quit)
        worker.failed.connect(thread.quit)
        thread.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)
        self._thread = thread
        self._worker = worker
        thread.start()

    @Slot(object)
    def _done(self, plan):
        self._thread = None
        self._worker = None
        self.finished.emit(plan)

    @Slot(str)
    def _fail(self, message):
        self._thread = None
        self._worker = None
        self.failed.emit(message)



    def review(self, port_name: str, plan):
        if self._review_thread is not None:
            return

        self.review_started.emit()

        thread = QThread(self)
        worker = _ReviewWorker(port_name, plan)
        worker.moveToThread(thread)

        thread.started.connect(worker.run)
        worker.progress.connect(self.review_progress)
        worker.finished.connect(self._review_done)
        worker.failed.connect(self._review_fail)

        worker.finished.connect(thread.quit)
        worker.failed.connect(thread.quit)
        thread.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)

        self._review_thread = thread
        self._review_worker = worker
        thread.start()

    @Slot(bool)
    def _review_done(self, approved: bool):
        self._review_thread = None
        self._review_worker = None
        self.review_finished.emit(approved)

    @Slot(str)
    def _review_fail(self, message: str):
        self._review_thread = None
        self._review_worker = None
        self.review_failed.emit(message)

    def sign(self, port_name, plan):
        if self._sign_thread is not None:
            return
        if plan.input_count < 1:
            self.sign_failed.emit("Transaktion enthält keine Inputs.")
            return
        self.sign_started.emit()
        thread = QThread(self)
        worker = _SignWorker(port_name, plan)
        worker.moveToThread(thread)
        thread.started.connect(worker.run)
        worker.progress.connect(self.sign_progress)
        worker.finished.connect(self._sign_done)
        worker.failed.connect(self._sign_fail)
        worker.finished.connect(thread.quit)
        worker.failed.connect(thread.quit)
        thread.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)
        self._sign_thread = thread
        self._sign_worker = worker
        thread.start()
    @Slot(object)
    def _sign_done(self,x):self._sign_thread=None;self._sign_worker=None;self.sign_finished.emit(x)
    @Slot(str)
    def _sign_fail(self,x):self._sign_thread=None;self._sign_worker=None;self.sign_failed.emit(x)


    def broadcast(self, server: str, signed):
        if self._broadcast_thread is not None:
            return

        if signed is None or not getattr(signed, "raw_hex", ""):
            self.broadcast_failed.emit("Keine signierte Transaktion vorhanden.")
            return

        self.broadcast_started.emit()

        thread = QThread(self)
        worker = _BroadcastWorker(server, signed)
        worker.moveToThread(thread)

        thread.started.connect(worker.run)
        worker.finished.connect(self._broadcast_done)
        worker.failed.connect(self._broadcast_fail)

        worker.finished.connect(thread.quit)
        worker.failed.connect(thread.quit)
        thread.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)

        self._broadcast_thread = thread
        self._broadcast_worker = worker
        thread.start()

    @Slot(str)
    def _broadcast_done(self, txid: str):
        self._broadcast_thread = None
        self._broadcast_worker = None
        self.broadcast_finished.emit(txid)

    @Slot(str)
    def _broadcast_fail(self, message: str):
        self._broadcast_thread = None
        self._broadcast_worker = None
        self.broadcast_failed.emit(message)


class _ReviewWorker(QObject):
    progress = Signal(str)
    finished = Signal(bool)
    failed = Signal(str)

    def __init__(self, port_name: str, plan):
        super().__init__()
        self._port_name = port_name
        self._plan = plan

    @Slot()
    def run(self):
        try:
            status = review_transaction(
                self._port_name,
                self._plan.recipient,
                self._plan.amount,
                self._plan.change,
                self._plan.fee,
            )

            if status == 2:
                raise RuntimeError(
                    "Hardware Wallet ist nicht im entsperrten Dashboard."
                )
            if status == 3:
                raise RuntimeError(
                    "Auf der Hardware läuft bereits eine andere Sicherheitsprüfung."
                )
            if status != 1:
                raise RuntimeError(
                    "Die Hardware hat den Transaktionsentwurf nicht angenommen."
                )

            self.progress.emit(
                "PIN auf der Hardware eingeben und Transaktion dort prüfen."
            )

            # No artificial confirmation timeout:
            # the desktop waits until the hardware explicitly answers.
            while True:
                time.sleep(0.6)
                result = get_transaction_result(self._port_name)

                if result == 0:
                    continue
                if result == 1:
                    self.finished.emit(True)
                    return
                if result == 2:
                    self.finished.emit(False)
                    return
                if result == 3:
                    continue

                raise RuntimeError(
                    f"Unbekannter Hardware-Status: {result}"
                )
        except Exception as exc:
            self.failed.emit(str(exc))

class _SignWorker(QObject):
    progress = Signal(str)
    finished = Signal(object)
    failed = Signal(str)

    def __init__(self, port, plan):
        super().__init__()
        self.port = port
        self.plan = plan

    @staticmethod
    def _raise_result(result: int) -> None:
        messages = {
            2: "Hardware konnte die Transaktion nicht signieren.",
            5: "Eine UTXO-Adresse gehört nicht zu dieser Wallet.",
            6: "Die Hardware konnte die Wallet-Schlüssel nicht laden.",
            7: "Die Hardware hat die Transaktionsdaten beim Signieren abgelehnt.",
            8: "Die kryptografische Signatur auf der Hardware ist fehlgeschlagen.",
        }
        raise RuntimeError(messages.get(result, f"Unbekannter Signaturstatus: {result}"))

    @Slot()
    def run(self):
        try:
            hash_prevouts, hash_sequence = _sign_context(self.plan)
            change_address = _change_address(self.plan) if self.plan.change else ""
            signed_inputs = []

            for position in range(self.plan.input_count):
                self.progress.emit(
                    f"Hardware signiert Input {position + 1} von {self.plan.input_count} …"
                )
                status = sign_transaction_input(
                    self.port,
                    self.plan,
                    position,
                    hash_prevouts,
                    hash_sequence,
                    change_address,
                )
                if status != 1:
                    raise RuntimeError({
                        2: "Hardware Wallet ist nicht im Dashboard.",
                        3: "Transaktion wurde noch nicht bestätigt.",
                        4: "Signatur läuft bereits.",
                    }.get(status, "Signaturanfrage abgelehnt."))

                while True:
                    time.sleep(0.25)
                    result, public_key, signature = get_sign_result(self.port)
                    if result == 0:
                        continue
                    if result == 1:
                        signed_inputs.append((public_key, signature))
                        break
                    self._raise_result(result)

            self.finished.emit(_build_signed(self.plan, signed_inputs))
        except Exception as exc:
            self.failed.emit(str(exc))



class _BroadcastWorker(QObject):
    finished = Signal(str)
    failed = Signal(str)

    def __init__(self, server: str, signed):
        super().__init__()
        self._server = server
        self._signed = signed

    @Slot()
    def run(self):
        try:
            txid = broadcast_transaction(
                self._server,
                self._signed.raw_hex,
            )

            expected = str(self._signed.txid).lower()
            if txid != expected:
                raise RuntimeError(
                    "Der vom Electrum-Server gemeldete TXID stimmt nicht "
                    "mit der lokal signierten Transaktion überein."
                )

            self.finished.emit(txid)
        except Exception as exc:
            self.failed.emit(str(exc))
