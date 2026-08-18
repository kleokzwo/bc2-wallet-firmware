from __future__ import annotations

from collections.abc import Callable

from PySide6.QtCore import Qt, Signal, QSize
from PySide6.QtGui import QIcon
GREEN = "#2EAD4A"
RED = "#C94B40"
ORANGE = "#F7931A"
TEXT = "#1E2025"
MUTED = "#6D7078"
BORDER = "#E2E4E8"

from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class DashboardPage(QWidget):
    """Dashboard presentation only.

    The page owns its widgets and visual state. Wallet, hardware and Electrum
    orchestration stay in MainWindow/services and update the page through the
    small public methods below.
    """

    show_transactions_requested = Signal()

    def __init__(
        self,
        icon_path: Callable[[str], str],
        electrum_server: str,
        parent: QWidget | None = None,
    ) -> None:
        super().__init__(parent)
        self._icon_path = icon_path
        self._device_connected = False
        self._network_state = "Nicht verbunden"
        self._sync_state = "noch nicht gestartet"
        self._server = electrum_server

        self._build_ui()
        self._refresh_status_icons()

    def _build_ui(self) -> None:
        self.setObjectName("Page")

        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(24)

        header = QHBoxLayout()
        title_box = QVBoxLayout()
        title_box.setSpacing(4)

        title = QLabel("DASHBOARD")
        title.setObjectName("PageTitle")
        subtitle = QLabel("Deine BC2 Wallet auf einen Blick.")
        subtitle.setObjectName("PageSubtitle")

        title_box.addWidget(title)
        title_box.addWidget(subtitle)
        header.addLayout(title_box, 1)

        status_box = QHBoxLayout()
        status_box.setSpacing(8)

        self._device_icon = QPushButton()
        self._device_icon.setObjectName("TopStatusIcon")
        self._device_icon.setToolTip("Hardware Wallet nicht verbunden")

        self._network_icon = QPushButton()
        self._network_icon.setObjectName("TopStatusIcon")
        self._network_icon.setToolTip("Netzwerk nicht verbunden")

        self._sync_icon = QPushButton()
        self._sync_icon.setObjectName("TopStatusIcon")
        self._sync_icon.setToolTip("Wallet noch nicht synchronisiert")

        for status in (self._device_icon, self._network_icon, self._sync_icon):
            status.setFlat(True)
            status.setCursor(Qt.ArrowCursor)
            status.setFocusPolicy(Qt.NoFocus)
            status.setFixedSize(30, 30)
            status.setIconSize(QSize(21, 21))
            status_box.addWidget(status)

        header.addLayout(status_box)
        outer.addLayout(header)

        balance = QFrame()
        balance.setObjectName("DashboardBalanceSection")
        balance_layout = QVBoxLayout(balance)
        balance_layout.setContentsMargins(0, 20, 0, 24)
        balance_layout.setSpacing(5)

        label = QLabel("Aktuelles Guthaben")
        label.setObjectName("DashboardBalanceLabel")
        self._confirmed_balance = QLabel("0.00000000 BC2")
        self._confirmed_balance.setObjectName("DashboardMainBalance")
        note = QLabel("Blockchain bestätigt")
        note.setObjectName("DashboardBalanceNote")

        balance_layout.addWidget(label)
        balance_layout.addWidget(self._confirmed_balance)
        balance_layout.addWidget(note)
        balance_layout.addSpacing(18)

        pending_row = QHBoxLayout()
        pending_label = QLabel("Unbestätigt")
        pending_label.setObjectName("DashboardPendingLabel")
        self._unconfirmed_balance = QLabel("0.00000000 BC2")
        self._unconfirmed_balance.setObjectName("DashboardPendingBalance")
        pending_row.addWidget(pending_label)
        pending_row.addWidget(self._unconfirmed_balance)
        pending_row.addStretch()
        balance_layout.addLayout(pending_row)

        self._unconfirmed_note = QLabel("Keine ausstehenden Transaktionen")
        self._unconfirmed_note.setObjectName("DashboardBalanceNote")
        balance_layout.addWidget(self._unconfirmed_note)

        outer.addWidget(balance)

        divider = QFrame()
        divider.setObjectName("DashboardDivider")
        divider.setFrameShape(QFrame.HLine)
        outer.addWidget(divider)

        tx_head = QHBoxLayout()
        tx_title = QLabel("Letzte Transaktionen")
        tx_title.setObjectName("DashboardSectionTitle")
        all_btn = QPushButton("Alle anzeigen")
        all_btn.setObjectName("DashboardTextButton")
        all_btn.clicked.connect(self.show_transactions_requested.emit)
        tx_head.addWidget(tx_title)
        tx_head.addStretch()
        tx_head.addWidget(all_btn)
        outer.addLayout(tx_head)

        self._transactions_container = QWidget()
        self._transactions_layout = QVBoxLayout(self._transactions_container)
        self._transactions_layout.setContentsMargins(0, 0, 0, 0)
        self._transactions_layout.setSpacing(0)

        outer.addWidget(self._transactions_container)
        outer.addStretch()

        self.set_transactions([])

    def set_transactions(self, entries) -> None:
        self._clear_transactions()

        entries = list(entries)[:5]

        if not entries:
            empty = QLabel("Noch keine Transaktionen vorhanden.")
            empty.setObjectName("DashboardEmptyText")
            self._transactions_layout.addWidget(empty)
            return

        for index, entry in enumerate(entries):
            self._transactions_layout.addWidget(
                self._transaction_row(entry)
            )

            if index < len(entries) - 1:
                divider = QFrame()
                divider.setFixedHeight(1)
                divider.setStyleSheet(
                    f"background:{BORDER}; border:none;"
                )
                self._transactions_layout.addWidget(divider)

    def _transaction_row(self, entry) -> QWidget:
        row = QWidget()
        row.setMinimumHeight(76)

        layout = QHBoxLayout(row)
        layout.setContentsMargins(4, 10, 4, 10)
        layout.setSpacing(14)

        if entry.direction == "incoming":
            symbol = "↓"
            title = "Empfangen"
            prefix = "+"
            amount_color = GREEN
        elif entry.direction == "outgoing":
            symbol = "↑"
            title = "Gesendet"
            prefix = "−"
            amount_color = RED
        else:
            symbol = "↔"
            title = "Eigenübertrag"
            prefix = ""
            amount_color = MUTED

        icon = QLabel(symbol)
        icon.setAlignment(Qt.AlignCenter)
        icon.setFixedSize(38, 38)
        icon.setStyleSheet(
            f"color:{amount_color};"
            f"border:1px solid {amount_color};"
            "border-radius:19px;"
            "font-size:20px;"
            "font-weight:800;"
        )

        info = QVBoxLayout()
        info.setSpacing(2)

        heading = QLabel(title)
        heading.setStyleSheet(
            f"color:{TEXT}; font-size:14px; font-weight:700;"
        )

        if entry.confirmed:
            state = f"Bestätigt · Block {entry.height}"
            state_color = GREEN
        else:
            state = "Unbestätigt"
            state_color = ORANGE

        status = QLabel(state)
        status.setStyleSheet(
            f"color:{state_color}; font-size:12px; font-weight:600;"
        )

        txid = QLabel(entry.txid)
        txid.setTextInteractionFlags(Qt.TextSelectableByMouse)
        txid.setStyleSheet(
            f"color:{MUTED}; font-size:11px;"
        )

        info.addWidget(heading)
        info.addWidget(status)
        info.addWidget(txid)

        amount = QLabel(
            f"{prefix}{entry.amount / 100_000_000:.8f} BC2"
        )
        amount.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        amount.setStyleSheet(
            f"color:{amount_color}; font-size:16px; font-weight:700;"
        )

        layout.addWidget(icon)
        layout.addLayout(info, 1)
        layout.addWidget(amount)

        return row

    def _clear_transactions(self) -> None:
        while self._transactions_layout.count():
            item = self._transactions_layout.takeAt(0)
            widget = item.widget()

            if widget is not None:
                widget.deleteLater()

    def set_device(self, connected: bool) -> None:
        self._device_connected = connected
        self._refresh_status_icons()

    def set_server(self, server: str) -> None:
        self._server = server

    def set_network_state(self, state: str, tooltip: str | None = None) -> None:
        self._network_state = state
        if tooltip:
            self._network_icon.setToolTip(tooltip)
        elif state == "Verbunden":
            self._network_icon.setToolTip("BC2 Netzwerk verbunden")
        else:
            self._network_icon.setToolTip("BC2 Netzwerk nicht verbunden")
        self._refresh_status_icons()

    def set_sync_state(self, state: str) -> None:
        self._sync_state = state
        self._refresh_status_icons()

    def set_balance(self, confirmed: str, unconfirmed: str, has_pending: bool) -> None:
        self._confirmed_balance.setText(confirmed)
        self._unconfirmed_balance.setText(unconfirmed)
        self._unconfirmed_note.setText(
            "Wartet auf Blockchain-Bestätigung"
            if has_pending
            else "Keine ausstehenden Transaktionen"
        )

    def _refresh_status_icons(self) -> None:
        network_ok = self._network_state == "Verbunden"
        sync_ok = self._sync_state.startswith("Aktuell")

        self._device_icon.setIcon(
            QIcon(self._icon_path("usb-green" if self._device_connected else "usb-gray"))
        )
        self._device_icon.setToolTip(
            "Hardware Wallet verbunden"
            if self._device_connected
            else "Hardware Wallet nicht verbunden"
        )

        self._network_icon.setIcon(
            QIcon(self._icon_path("globe-green" if network_ok else "globe-gray"))
        )
        self._sync_icon.setIcon(
            QIcon(self._icon_path("sync-green" if sync_ok else "sync-gray"))
        )
        self._sync_icon.setToolTip(
            "Wallet synchronisiert" if sync_ok else "Wallet noch nicht synchronisiert"
        )
