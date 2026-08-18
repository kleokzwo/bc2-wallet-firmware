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
    # Important: keep amount as text so Decimal can parse it exactly.
    send_requested = Signal(str, str)
    review_requested = Signal()

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

        self._preview = QFrame()
        self._preview.setObjectName("ResultPanel")
        self._preview.setVisible(False)

        preview_layout = QVBoxLayout(self._preview)
        preview_layout.setContentsMargins(18, 16, 18, 16)
        preview_layout.setSpacing(6)

        preview_title = QLabel("Transaktionsentwurf")
        preview_title.setObjectName("CardTitle")

        self._preview_text = QLabel("")
        self._preview_text.setObjectName("BodyText")
        self._preview_text.setWordWrap(True)
        self._preview_text.setTextInteractionFlags(Qt.TextSelectableByMouse)

        preview_layout.addWidget(preview_title)
        preview_layout.addWidget(self._preview_text)

        self._review_button = QPushButton("Auf Hardware prüfen")
        self._review_button.setObjectName("PrimaryButton")
        self._review_button.clicked.connect(self.review_requested)
        preview_layout.addWidget(
            self._review_button,
            alignment=Qt.AlignLeft,
        )

        self._review_status = QLabel("")
        self._review_status.setObjectName("SmallMuted")
        self._review_status.setWordWrap(True)
        preview_layout.addWidget(self._review_status)

        layout.addWidget(self._preview)
        layout.addStretch()

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())

    def _submit(self) -> None:
        self._error.setText("")
        self._preview.setVisible(False)

        address = self._address_input.text().strip()
        amount_text = self._amount_input.text().strip().replace(",", ".")

        if not address:
            self._error.setText("Bitte eine Empfängeradresse eingeben.")
            return

        if len(address) < 20:
            self._error.setText("Die Empfängeradresse ist zu kurz.")
            return

        if not amount_text:
            self._error.setText("Bitte einen Betrag eingeben.")
            return

        # Do not convert to float here. Send the exact string to SendService.
        self.send_requested.emit(address, amount_text)

    def show_hardware_required(self) -> None:
        self.show_prepare_failed(
            "Hardware Wallet nicht verbunden. "
            "Senden ist ohne Hardware-Bestätigung nicht möglich."
        )

    def show_preparing(self) -> None:
        self._error.setText("")
        self._preview.setVisible(False)
        self._next_button.setEnabled(False)
        self._next_button.setText("UTXOs werden geladen …")

    def show_plan(self, plan) -> None:
        self._error.setText("")
        self._next_button.setEnabled(True)
        self._next_button.setText("Neu berechnen")
        self._review_button.setEnabled(True)
        self._review_button.setText("Auf Hardware prüfen")
        self._review_status.setText("")

        self._preview_text.setText(
            f"Empfänger: {plan.recipient}\n"
            f"Betrag: {plan.amount / 100_000_000:.8f} BC2\n"
            f"Netzwerkgebühr: {plan.fee / 100_000_000:.8f} BC2 "
            f"({plan.fee_rate} sat/vB)\n"
            f"Wechselgeld: {plan.change / 100_000_000:.8f} BC2\n"
            f"Eingänge: {plan.input_count} · geschätzt {plan.estimated_vbytes} vB\n\n"
            "Noch nicht signiert und noch nicht gesendet."
        )
        self._preview.setVisible(True)

    def show_review_started(self) -> None:
        self._error.setText("")
        self._review_button.setEnabled(False)
        self._review_button.setText("Warte auf Hardware …")
        self._review_status.setText(
            "Transaktionsentwurf wird an die Hardware übertragen."
        )

    def show_review_progress(self, message: str) -> None:
        self._review_status.setText(message)

    def show_review_result(self, approved: bool) -> None:
        self._review_button.setEnabled(True)
        if approved:
            self._review_button.setText("✓ Auf Hardware bestätigt")
            self._review_status.setText(
                "Transaktion wurde auf der Hardware bestätigt. "
                "Noch nicht signiert und noch nicht gesendet."
            )
        else:
            self._review_button.setText("Erneut auf Hardware prüfen")
            self._review_status.setText(
                "Transaktion wurde auf der Hardware abgelehnt."
            )

    def show_review_failed(self, message: str) -> None:
        self._review_button.setEnabled(True)
        self._review_button.setText("Auf Hardware prüfen")
        self._review_status.setText("")
        self._error.setText(message)

    def show_prepare_failed(self, message: str) -> None:
        self._next_button.setEnabled(True)
        self._next_button.setText("Weiter")
        self._preview.setVisible(False)
        self._error.setText(message)

    def clear_error(self) -> None:
        self._error.setText("")

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
