from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QVBoxLayout,
    QWidget,
)


class TransactionPage(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        # Page header
        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel("TRANSAKTIONEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Hier erscheinen deine BC2 Transaktionen nach der Wallet-Synchronisierung."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())

        outer.addLayout(head)

        # Transactions content
        card = QFrame()
        card.setObjectName("Card")

        layout = QVBoxLayout(card)
        layout.setContentsMargins(24, 22, 24, 22)

        title = QLabel("Transaktionsverlauf")
        title.setObjectName("SectionTitle")

        empty = QLabel(
            "Keine Transaktionen vorhanden\n\n"
            "Die Blockchain-Synchronisierung wird in einem folgenden Sprint an diese Seite angebunden."
        )
        empty.setObjectName("EmptyState")
        empty.setAlignment(Qt.AlignCenter)
        empty.setWordWrap(True)

        layout.addWidget(title)
        layout.addStretch()
        layout.addWidget(empty)
        layout.addStretch()

        outer.addWidget(card, 1)

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
