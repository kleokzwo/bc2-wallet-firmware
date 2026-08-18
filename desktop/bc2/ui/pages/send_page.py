from __future__ import annotations

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QDoubleValidator
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class SendPage(QWidget):
    send_requested = Signal(str, float)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel("SENDEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Transaktionen werden auf dem Desktop vorbereitet und müssen auf der Hardware geprüft werden."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())
        outer.addLayout(head)

        card = QFrame()
        card.setObjectName("Card")

        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(12)

        title = QLabel("BC2 senden")
        title.setObjectName("SectionTitle")
        layout.addWidget(title)

        layout.addWidget(self._form_label("Empfängeradresse"))

        self._address_input = QLineEdit()
        self._address_input.setPlaceholderText("BC2-Adresse eingeben")
        self._address_input.setObjectName("Input")
        layout.addWidget(self._address_input)

        layout.addWidget(self._form_label("Betrag"))

        self._amount_input = QLineEdit()
        self._amount_input.setPlaceholderText("0.00000000")
        self._amount_input.setObjectName("Input")

        validator = QDoubleValidator(0.0, 999999999.0, 8, self)
        validator.setNotation(QDoubleValidator.StandardNotation)
        self._amount_input.setValidator(validator)

        layout.addWidget(self._amount_input)

        self._error = QLabel("")
        self._error.setObjectName("ErrorText")
        self._error.setWordWrap(True)
        layout.addWidget(self._error)

        self._next_button = QPushButton("Weiter")
        self._next_button.setObjectName("PrimaryButton")
        self._next_button.clicked.connect(self._submit)
        layout.addWidget(self._next_button, alignment=Qt.AlignLeft)

        layout.addStretch()

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())

    def _form_label(self, text: str) -> QLabel:
        label = QLabel(text)
        label.setObjectName("FormLabel")
        return label

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

    def _safety_banner(self) -> QWidget:
        banner = QFrame()
        banner.setObjectName("SafetyBanner")

        row = QHBoxLayout(banner)
        row.setContentsMargins(18, 13, 18, 13)

        icon = QLabel("●")
        icon.setObjectName("SafetyIcon")
        icon.setFixedWidth(24)

        text = QVBoxLayout()

        title = QLabel("Sicherheit zuerst")
        title.setObjectName("SafetyTitle")

        detail = QLabel(
            "PIN, Seed und private Schlüssel bleiben ausschließlich auf der Hardware. "
            "Die Geräte-PIN besteht aus genau 4 Ziffern."
        )
        detail.setObjectName("SafetyText")
        detail.setWordWrap(True)

        text.addWidget(title)
        text.addWidget(detail)

        row.addWidget(icon)
        row.addLayout(text, 1)
        return banner

    def _submit(self) -> None:
        self._error.setText("")

        address = self._address_input.text().strip()
        amount_text = self._amount_input.text().strip().replace(",", ".")

        if not address:
            self._error.setText("Bitte eine Empfängeradresse eingeben.")
            return

        if len(address) < 20:
            self._error.setText("Die Empfängeradresse ist zu kurz.")
            return

        try:
            amount = float(amount_text)
        except ValueError:
            self._error.setText("Bitte einen gültigen Betrag eingeben.")
            return

        if amount <= 0:
            self._error.setText("Der Betrag muss größer als 0 sein.")
            return

        self.send_requested.emit(address, amount)

    def show_hardware_required(self) -> None:
        self._error.setText(
            "Hardware Wallet nicht verbunden. "
            "Senden ist ohne Hardware-Bestätigung nicht möglich."
        )

    def clear_error(self) -> None:
        self._error.setText("")
