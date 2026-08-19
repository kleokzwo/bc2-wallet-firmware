from __future__ import annotations

import socket
import ssl
from dataclasses import dataclass

from PySide6.QtCore import QObject, QThread, Signal, Slot

from .electrum_service import (
    _rpc,
    address_to_scriptpubkey,
    electrum_scripthash,
)


@dataclass(frozen=True)
class TransactionEntry:
    txid: str
    direction: str
    amount: int
    height: int

    @property
    def confirmed(self) -> bool:
        return self.height > 0


def _read_varint(raw: bytes, offset: int) -> tuple[int, int]:
    if offset >= len(raw):
        raise ValueError("Transaktion ist abgeschnitten.")

    first = raw[offset]
    offset += 1

    if first < 0xFD:
        return first, offset

    if first == 0xFD:
        size = 2
    elif first == 0xFE:
        size = 4
    else:
        size = 8

    if offset + size > len(raw):
        raise ValueError("Ungültiger CompactSize-Wert.")

    value = int.from_bytes(raw[offset:offset + size], "little")
    return value, offset + size


def _parse_transaction(raw_hex: str) -> dict:
    try:
        raw = bytes.fromhex(raw_hex)
    except ValueError as exc:
        raise ValueError("Electrum lieferte eine ungültige Raw Transaction.") from exc

    if len(raw) < 10:
        raise ValueError("Raw Transaction ist zu kurz.")

    offset = 4  # version

    segwit = (
        offset + 2 <= len(raw)
        and raw[offset] == 0x00
        and raw[offset + 1] != 0x00
    )
    if segwit:
        offset += 2

    input_count, offset = _read_varint(raw, offset)
    inputs = []

    for _ in range(input_count):
        if offset + 36 > len(raw):
            raise ValueError("Transaktions-Input ist abgeschnitten.")

        prev_txid = raw[offset:offset + 32][::-1].hex()
        offset += 32

        prev_index = int.from_bytes(raw[offset:offset + 4], "little")
        offset += 4

        script_length, offset = _read_varint(raw, offset)

        if offset + script_length + 4 > len(raw):
            raise ValueError("Transaktions-Input-Script ist abgeschnitten.")

        offset += script_length
        sequence = int.from_bytes(raw[offset:offset + 4], "little")
        offset += 4

        inputs.append({
            "txid": prev_txid,
            "vout": prev_index,
            "sequence": sequence,
        })

    output_count, offset = _read_varint(raw, offset)
    outputs = []

    for _ in range(output_count):
        if offset + 8 > len(raw):
            raise ValueError("Transaktions-Output ist abgeschnitten.")

        value = int.from_bytes(raw[offset:offset + 8], "little")
        offset += 8

        script_length, offset = _read_varint(raw, offset)

        if offset + script_length > len(raw):
            raise ValueError("Transaktions-Output-Script ist abgeschnitten.")

        script = bytes(raw[offset:offset + script_length])
        offset += script_length

        outputs.append({
            "value": value,
            "script": script,
        })

    if segwit:
        for _ in range(input_count):
            item_count, offset = _read_varint(raw, offset)

            for _ in range(item_count):
                item_length, offset = _read_varint(raw, offset)

                if offset + item_length > len(raw):
                    raise ValueError("Witness-Daten sind abgeschnitten.")

                offset += item_length

    if offset + 4 > len(raw):
        raise ValueError("Transaktions-Locktime fehlt.")

    return {
        "inputs": inputs,
        "outputs": outputs,
    }


def fetch_transactions(
    server: str,
    addresses,
    timeout: float = 10.0,
) -> list[TransactionEntry]:
    """
    Fast KISS history sync:
    - fetch history for each known wallet address
    - fetch each wallet transaction exactly once
    - classify incoming/outgoing locally
    - no recursive loading of previous transactions
    """
    if ":" not in server:
        raise ValueError("Electrum Server muss host:port sein.")

    addresses = list(
        dict.fromkeys(
            str(address).strip()
            for address in addresses
            if str(address).strip()
        )
    )

    if not addresses:
        return []

    wallet_scripts = {
        address_to_scriptpubkey(address)
        for address in addresses
    }

    host, port_text = server.rsplit(":", 1)
    port = int(port_text)

    history: dict[str, int] = {}
    transactions: dict[str, dict] = {}

    context = ssl.create_default_context()

    with socket.create_connection(
        (host, port),
        timeout=timeout,
    ) as raw_socket:
        with context.wrap_socket(
            raw_socket,
            server_hostname=host,
        ) as socket_connection:
            socket_connection.settimeout(timeout)

            with socket_connection.makefile("rwb") as file:
                request_id = 1

                _rpc(
                    file,
                    request_id,
                    "server.version",
                    ["BC2 Cold Wallet", "1.4"],
                )
                request_id += 1

                # 1) Collect all unique wallet TXIDs.
                for address in addresses:
                    rows = _rpc(
                        file,
                        request_id,
                        "blockchain.scripthash.get_history",
                        [electrum_scripthash(address)],
                    ) or []
                    request_id += 1

                    for row in rows:
                        txid = str(
                            row.get("tx_hash", "")
                        ).strip().lower()

                        if len(txid) != 64:
                            continue

                        height = int(row.get("height", 0))
                        old_height = history.get(txid)

                        if old_height is None or height > old_height:
                            history[txid] = height

                # 2) Fetch every wallet transaction once.
                for txid in history:
                    raw_hex = _rpc(
                        file,
                        request_id,
                        "blockchain.transaction.get",
                        [txid],
                    )
                    request_id += 1

                    if not isinstance(raw_hex, str):
                        raise RuntimeError(
                            f"Electrum lieferte für {txid} keine Raw Transaction."
                        )

                    transactions[txid] = _parse_transaction(raw_hex)

    # 3) Index every output belonging to this wallet.
    #
    # Only transactions already present in wallet history are used.
    # No additional Electrum requests are made here.
    wallet_outputs: dict[tuple[str, int], int] = {}

    for txid, transaction in transactions.items():
        for output_index, output in enumerate(transaction["outputs"]):
            if output["script"] in wallet_scripts:
                wallet_outputs[(txid, output_index)] = output["value"]

    entries: list[TransactionEntry] = []

    # 4) Classify locally.
    for txid, height in history.items():
        transaction = transactions[txid]

        wallet_received = sum(
            output["value"]
            for output in transaction["outputs"]
            if output["script"] in wallet_scripts
        )

        wallet_spent = sum(
            wallet_outputs.get(
                (
                    tx_input["txid"],
                    tx_input["vout"],
                ),
                0,
            )
            for tx_input in transaction["inputs"]
        )

        if wallet_spent == 0 and wallet_received > 0:
            direction = "incoming"
            amount = wallet_received

        elif wallet_spent > 0:
            external_sent = sum(
                output["value"]
                for output in transaction["outputs"]
                if output["script"] not in wallet_scripts
            )

            if external_sent > 0:
                direction = "outgoing"
                amount = external_sent
            else:
                direction = "self"
                amount = 0

        else:
            continue

        entries.append(
            TransactionEntry(
                txid=txid,
                direction=direction,
                amount=amount,
                height=height,
            )
        )

    # Unconfirmed first, then newest confirmed blocks.
    entries.sort(
        key=lambda entry: (
            0 if not entry.confirmed else 1,
            -entry.height if entry.confirmed else 0,
        )
    )

    return entries


class _TransactionWorker(QObject):
    finished = Signal(object)
    failed = Signal(str)

    def __init__(self, server: str, addresses):
        super().__init__()
        self._server = server
        self._addresses = list(addresses)

    @Slot()
    def run(self) -> None:
        try:
            self.finished.emit(
                fetch_transactions(
                    self._server,
                    self._addresses,
                )
            )
        except Exception as exc:
            self.failed.emit(str(exc))


class TransactionService(QObject):
    started = Signal()
    finished = Signal(object)
    failed = Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._thread = None
        self._worker = None

    def sync(self, server: str, addresses) -> None:
        if self._thread is not None:
            return

        self.started.emit()

        thread = QThread(self)
        worker = _TransactionWorker(server, addresses)
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
    def _done(self, entries) -> None:
        self._thread = None
        self._worker = None
        self.finished.emit(entries)

    @Slot(str)
    def _fail(self, message: str) -> None:
        self._thread = None
        self._worker = None
        self.failed.emit(message)
