from __future__ import annotations

from dataclasses import dataclass
from decimal import Decimal, InvalidOperation

from PySide6.QtCore import QObject, QThread, Signal, Slot

from .electrum_service import fetch_utxos
from bc2.device.discovery import review_transaction, get_transaction_result
import time

SATOSHIS_PER_BC2 = 100_000_000
DEFAULT_FEE_RATE = 2
DUST_LIMIT = 546


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

    def __init__(self, parent=None):
        super().__init__(parent)
        self._thread = None
        self._worker = None
        self._review_thread = None
        self._review_worker = None

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
