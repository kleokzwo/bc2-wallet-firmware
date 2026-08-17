from __future__ import annotations

import time

from PySide6.QtCore import QObject, QThread, Signal, Slot

from bc2.device.discovery import begin_receive_address, get_receive_result


class _ReceiveWorker(QObject):
    progress = Signal(str)
    finished = Signal(int, object)
    failed = Signal(str)

    def __init__(self, port_name: str):
        super().__init__()
        self._port_name = port_name

    @Slot()
    def run(self) -> None:
        try:
            accepted = begin_receive_address(self._port_name)
            if accepted == 2:
                self.failed.emit(
                    "Die Hardware Wallet ist noch nicht entsperrt oder nicht bereit."
                )
                return
            if accepted == 3:
                self.failed.emit(
                    "Auf der Hardware läuft bereits eine andere Bestätigung."
                )
                return
            if accepted != 1:
                self.failed.emit("Die Hardware hat die Adressanforderung abgelehnt.")
                return

            self.progress.emit(
                "Gib deine 4-stellige PIN auf der Hardware ein und prüfe dort die Adresse."
            )

            deadline = time.monotonic() + 120.0
            last_error = None
            while time.monotonic() < deadline:
                time.sleep(0.7)
                try:
                    status, address = get_receive_result(self._port_name)
                    last_error = None
                except (TimeoutError, OSError) as exc:
                    # The E-Paper refresh can temporarily block the firmware loop
                    # after physical confirmation. The approved address remains
                    # stored on-device, so a short USB pause is retryable.
                    last_error = str(exc)
                    self.progress.emit(
                        "Adresse bestätigt – warte auf Übertragung vom Gerät …"
                    )
                    continue
                if status == 0:
                    continue
                if status == 1 and address:
                    self.finished.emit(status, address)
                    return
                if status == 2:
                    self.finished.emit(status, None)
                    return
                if status == 4:
                    self.failed.emit(
                        "Die Hardware konnte die Empfangsadresse nicht ableiten. "
                        "Firmware v0.40.0 enthält dafür eine korrigierte secp256k1-Hardwareableitung."
                    )
                    return
                if status == 3:
                    # NONE can briefly appear only before a request is registered.
                    continue

            detail = f" Letzter USB-Fehler: {last_error}" if last_error else ""
            self.failed.emit(
                "Zeitüberschreitung. Die bestätigte Adresse konnte nicht vom Gerät gelesen werden."
                + detail
            )
        except Exception as exc:
            self.failed.emit(str(exc))


class ReceiveService(QObject):
    started = Signal()
    progress = Signal(str)
    finished = Signal(int, object)
    failed = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._thread = None
        self._worker = None

    def request(self, port_name: str) -> None:
        if self._thread is not None:
            return

        thread = QThread(self)
        worker = _ReceiveWorker(port_name)
        worker.moveToThread(thread)

        thread.started.connect(worker.run)
        worker.progress.connect(self.progress)
        worker.finished.connect(self._done)
        worker.failed.connect(self._failed)
        worker.finished.connect(thread.quit)
        worker.failed.connect(thread.quit)
        thread.finished.connect(worker.deleteLater)
        thread.finished.connect(thread.deleteLater)

        self._thread = thread
        self._worker = worker
        self.started.emit()
        thread.start()

    @Slot(int, object)
    def _done(self, status: int, address) -> None:
        self._thread = None
        self._worker = None
        self.finished.emit(status, address)

    @Slot(str)
    def _failed(self, message: str) -> None:
        self._thread = None
        self._worker = None
        self.failed.emit(message)
