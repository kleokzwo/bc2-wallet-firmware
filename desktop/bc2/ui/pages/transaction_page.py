from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QScrollArea,
    QVBoxLayout,
    QWidget,
)


GREEN = "#2EAD4A"
RED = "#C94B40"
ORANGE = "#F7931A"
TEXT = "#1E2025"
MUTED = "#6D7078"
BORDER = "#E2E4E8"


class TransactionPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._entries = []
        self._page = 0
        self._page_size = 10
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel("TRANSAKTIONEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Alle eingehenden und ausgehenden BC2 Transaktionen deiner Wallet."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())

        outer.addLayout(head)

        card = QFrame()
        card.setObjectName("TransactionContainer")
        card.setStyleSheet("""
            QFrame#TransactionContainer {
                background: transparent;
                border: none;
            }
        """)

        layout = QVBoxLayout(card)
        layout.setContentsMargins(24, 22, 24, 22)
        layout.setSpacing(14)

        header = QHBoxLayout()

        title = QLabel("Transaktionsverlauf")
        title.setObjectName("TransactionHistoryTitle")
        title.setStyleSheet("""
            QLabel#TransactionHistoryTitle {
                color: #1E2025;
                font-size: 18px;
                font-weight: 600;
            }
        """)

        self._sync_state = QLabel("Noch nicht synchronisiert")
        self._sync_state.setObjectName("SmallMuted")
        self._sync_state.setAlignment(Qt.AlignRight | Qt.AlignVCenter)

        header.addWidget(title)
        header.addStretch()
        header.addWidget(self._sync_state)

        layout.addLayout(header)

        self._scroll = QScrollArea()
        self._scroll.setWidgetResizable(True)
        self._scroll.setFrameShape(QFrame.NoFrame)
        self._scroll.setStyleSheet(
            "QScrollArea { background: transparent; }"
        )

        self._content = QWidget()
        self._content.setStyleSheet("background: transparent;")

        self._rows = QVBoxLayout(self._content)
        self._rows.setContentsMargins(0, 0, 0, 0)
        self._rows.setSpacing(0)

        self._scroll.setWidget(self._content)
        layout.addWidget(self._scroll, 1)

        pagination = QHBoxLayout()

        self._prev_button = QPushButton("← Zurück")
        self._prev_button.setObjectName("OutlineButton")
        self._prev_button.clicked.connect(self._previous_page)

        self._page_label = QLabel("Seite 1 / 1")
        self._page_label.setObjectName("SmallMuted")
        self._page_label.setAlignment(Qt.AlignCenter)

        self._next_button = QPushButton("Weiter →")
        self._next_button.setObjectName("OutlineButton")
        self._next_button.clicked.connect(self._next_page)

        pagination.addWidget(self._prev_button)
        pagination.addStretch()
        pagination.addWidget(self._page_label)
        pagination.addStretch()
        pagination.addWidget(self._next_button)

        layout.addLayout(pagination)

        outer.addWidget(card, 1)

        self.show_loading()

        pagination_button_style = """
            QPushButton {
                background-color: #8E159D;
                color: white;
                border: none;
                border-radius: 8px;
                padding: 9px 18px;
                font-weight: 600;
            }

            QPushButton:hover {
                background-color: #761183;
            }

            QPushButton:pressed {
                background-color: #64106F;
            }

            QPushButton:disabled {
                background-color: #D8D8DC;
                color: #FFFFFF;
            }
        """

        self._prev_button.setStyleSheet(pagination_button_style)
        self._next_button.setStyleSheet(pagination_button_style)

    def show_loading(self) -> None:
        self._clear_rows()
        self._sync_state.setText("Synchronisiere …")
        self._page_label.setText("Lade …")
        self._prev_button.setEnabled(False)
        self._next_button.setEnabled(False)

        label = QLabel(
            "Transaktionen werden von Electrum geladen …"
        )
        label.setObjectName("EmptyState")
        label.setAlignment(Qt.AlignCenter)
        label.setWordWrap(True)

        self._rows.addStretch()
        self._rows.addWidget(label)
        self._rows.addStretch()

    def show_transactions(self, entries) -> None:
        self._entries = list(entries)
        self._page = 0
        self._render_page()

    def _render_page(self) -> None:
        self._clear_rows()

        if not self._entries:
            self._sync_state.setText("Aktuell")
            self._page_label.setText("Seite 1 / 1")
            self._prev_button.setEnabled(False)
            self._next_button.setEnabled(False)

            label = QLabel(
                "Keine Transaktionen für die bekannten Wallet-Adressen gefunden."
            )
            label.setObjectName("EmptyState")
            label.setAlignment(Qt.AlignCenter)
            label.setWordWrap(True)

            self._rows.addStretch()
            self._rows.addWidget(label)
            self._rows.addStretch()
            return

        total_pages = max(
            1,
            (len(self._entries) + self._page_size - 1) // self._page_size,
        )

        if self._page >= total_pages:
            self._page = total_pages - 1

        start_index = self._page * self._page_size
        end_index = start_index + self._page_size
        visible_entries = self._entries[start_index:end_index]

        self._sync_state.setText(f"Aktuell · {len(self._entries)}")
        self._page_label.setText(
            f"Seite {self._page + 1} / {total_pages}"
        )
        self._prev_button.setEnabled(self._page > 0)
        self._next_button.setEnabled(self._page + 1 < total_pages)

        for index, entry in enumerate(visible_entries):
            self._rows.addWidget(self._transaction_row(entry))

            if index < len(visible_entries) - 1:
                divider = QFrame()
                divider.setFixedHeight(1)
                divider.setStyleSheet(
                    f"background:{BORDER}; border:none;"
                )
                self._rows.addWidget(divider)

        self._rows.addStretch()

    def _previous_page(self) -> None:
        if self._page <= 0:
            return

        self._page -= 1
        self._render_page()
        self._scroll.verticalScrollBar().setValue(0)

    def _next_page(self) -> None:
        total_pages = max(
            1,
            (len(self._entries) + self._page_size - 1) // self._page_size,
        )

        if self._page + 1 >= total_pages:
            return

        self._page += 1
        self._render_page()
        self._scroll.verticalScrollBar().setValue(0)

    def show_error(self, message: str) -> None:
        self._clear_rows()
        self._sync_state.setText("Sync fehlgeschlagen")
        self._page_label.setText("—")
        self._prev_button.setEnabled(False)
        self._next_button.setEnabled(False)

        label = QLabel(
            f"Transaktionen konnten nicht geladen werden.\n\n{message}"
        )
        label.setObjectName("ErrorText")
        label.setAlignment(Qt.AlignCenter)
        label.setWordWrap(True)

        self._rows.addStretch()
        self._rows.addWidget(label)
        self._rows.addStretch()

    def _transaction_row(self, entry) -> QWidget:
        row = QWidget()
        row.setMinimumHeight(82)

        layout = QHBoxLayout(row)
        layout.setContentsMargins(8, 12, 8, 12)
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
        info.setSpacing(3)

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

    def _clear_rows(self) -> None:
        while self._rows.count():
            item = self._rows.takeAt(0)
            widget = item.widget()

            if widget is not None:
                widget.deleteLater()

    def _security_badge(self) -> QWidget:
        frame = QFrame()
        frame.setObjectName("SecurityBadge")
        frame.setFixedWidth(250)

        row = QHBoxLayout(frame)
        row.setContentsMargins(14, 11, 14, 11)

        icon = QLabel("✓")
        icon.setObjectName("SecurityIcon")
        icon.setAlignment(Qt.AlignCenter)
        icon.setFixedSize(34, 34)

        text = QVBoxLayout()
        text.setSpacing(0)

        primary = QLabel("Sicher & Offline")
        primary.setObjectName("SecurityPrimary")

        secondary = QLabel("Schlüssel bleiben auf Hardware")
        secondary.setObjectName("SecuritySecondary")

        text.addWidget(primary)
        text.addWidget(secondary)

        row.addWidget(icon)
        row.addLayout(text, 1)

        return frame
