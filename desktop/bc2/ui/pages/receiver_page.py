from __future__ import annotations

from io import BytesIO

import qrcode

from PySide6.QtCore import Qt, Signal, QTimer
from PySide6.QtGui import QGuiApplication, QPixmap
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class ReceivePage(QWidget):
    request_receive_requested = Signal()

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

        title_label = QLabel("EMPFANGEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Eine neue BC2 Empfangsadresse wird erst nach Bestätigung auf der Hardware angezeigt."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())
        outer.addLayout(head)

        # Receive content
        card = QFrame()
        card.setObjectName("Card")
        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(16)

        title = QLabel("BC2 empfangen")
        title.setObjectName("SectionTitle")

        self._state = QLabel(
            "Verbinde deine Hardware Wallet, um eine Empfangsadresse anzufordern."
        )
        self._state.setObjectName("BodyText")
        self._state.setWordWrap(True)

        self._request_button = QPushButton("Empfangsadresse anfordern")
        self._request_button.setObjectName("PrimaryButton")
        self._request_button.clicked.connect(self.request_receive_requested)

        self._result = QFrame()
        self._result.setObjectName("ResultPanel")
        rr = QVBoxLayout(self._result)
        rr.setContentsMargins(18, 16, 18, 16)

        self._result_title = QLabel("Noch keine Adresse")
        self._result_title.setObjectName("CardTitle")

        self._result_text = QLabel(
            "Aus Sicherheitsgründen zeigt der Desktop keine erfundene Adresse an."
        )
        self._result_text.setObjectName("SmallMuted")
        self._result_text.setWordWrap(True)

        self._qr = QLabel()
        self._qr.setObjectName("ReceiveQr")
        self._qr.setAlignment(Qt.AlignCenter)
        self._qr.setFixedSize(220, 220)
        self._qr.setVisible(False)

        self._copy_button = QPushButton("Adresse kopieren")
        self._copy_button.setObjectName("OutlineButton")
        self._copy_button.setVisible(False)
        self._copy_button.clicked.connect(self._copy_receive_address)

        rr.addWidget(self._result_title)
        rr.addWidget(self._qr, alignment=Qt.AlignLeft)
        rr.addWidget(self._result_text)
        rr.addWidget(self._copy_button, alignment=Qt.AlignLeft)

        layout.addWidget(title)
        layout.addWidget(self._state)
        layout.addWidget(self._request_button, alignment=Qt.AlignLeft)
        layout.addWidget(self._result)
        layout.addStretch()

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())

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

    def set_device_connected(self, connected: bool) -> None:
        if connected:
            self._state.setText(
                "Hardware Wallet verbunden. Empfangsadresse muss auf dem Gerät bestätigt werden."
            )
        else:
            self._state.setText("Hardware Wallet nicht verbunden.")

    def show_hardware_required(self) -> None:
        self._result_title.setText("Hardware Wallet erforderlich")
        self._result_text.setText(
            "Schließe zuerst deine BC2 Hardware Wallet an."
        )
        self._copy_button.setVisible(False)

    def show_wallet_not_ready(self) -> None:
        self._result_title.setText("Wallet noch nicht eingerichtet")
        self._result_text.setText(
            "Richte zuerst deine Wallet vollständig auf der Hardware ein."
        )
        self._copy_button.setVisible(False)

    def show_wallet_locked(self) -> None:
        self._result_title.setText("Wallet ist gesperrt")
        self._result_text.setText(
            "Entsperre zuerst die Hardware Wallet mit deiner 4-stelligen PIN."
        )
        self._copy_button.setVisible(False)

    def show_request_started(self) -> None:
        self._request_button.setEnabled(False)
        self._request_button.setText("Warte auf Hardware …")
        self._set_qr(None)
        self._result_title.setText("Bestätigung auf Hardware erforderlich")
        self._result_text.setText(
            "Die Empfangsadresse wird ausschließlich auf der Hardware erzeugt."
        )
        self._copy_button.setVisible(False)

    def show_progress(self, message: str) -> None:
        self._result_title.setText("Adresse auf Hardware prüfen")
        self._result_text.setText(message)

    def show_finished(self, status: int, address) -> str | None:
        self._request_button.setEnabled(True)
        self._request_button.setText("Neue Empfangsadresse anfordern")

        if status == 1 and address:
            address_text = str(address)
            self._result_title.setText("Empfangsadresse bestätigt")
            self._set_qr(address_text)
            self._result_text.setText(f"Adresse: {address_text}")
            self._result_text.setTextInteractionFlags(Qt.TextSelectableByMouse)
            self._copy_button.setVisible(True)
            self._state.setText(
                "Diese Adresse wurde auf deiner BC2 Hardware Wallet geprüft und bestätigt."
            )
            return address_text

        self._set_qr(None)
        self._result_title.setText("Adresse nicht freigegeben")
        self._result_text.setText(
            "Die Hardware Wallet hat die Adresse abgelehnt oder konnte sie nicht sicher erzeugen. "
            "Bei einem technischen Ableitungsfehler wird jetzt eine genauere Meldung ausgegeben."
        )
        self._copy_button.setVisible(False)
        return None

    def show_failed(self, message: str) -> None:
        self._request_button.setEnabled(True)
        self._request_button.setText("Empfangsadresse anfordern")
        self._set_qr(None)
        self._result_title.setText("Empfangen nicht möglich")
        self._result_text.setText(message)
        self._copy_button.setVisible(False)

    def _set_qr(self, address: str | None) -> None:
        self._qr.clear()
        self._qr.setVisible(False)

        if not address:
            return

        qr = qrcode.QRCode(
            version=None,
            error_correction=qrcode.constants.ERROR_CORRECT_M,
            box_size=7,
            border=3,
        )
        qr.add_data(address)
        qr.make(fit=True)

        image = qr.make_image(fill_color="black", back_color="white")
        buffer = BytesIO()
        image.save(buffer, format="PNG")

        pixmap = QPixmap()
        if pixmap.loadFromData(buffer.getvalue(), "PNG"):
            self._qr.setPixmap(
                pixmap.scaled(
                    210,
                    210,
                    Qt.KeepAspectRatio,
                    Qt.SmoothTransformation,
                )
            )
            self._qr.setVisible(True)

    def _copy_receive_address(self) -> None:
        address = self._result_text.text().strip()
        if address.startswith("Adresse:"):
            address = address.split(":", 1)[1].strip()

        if address:
            QGuiApplication.clipboard().setText(address)
            self._copy_button.setText("✓ Kopiert")
            QTimer.singleShot(
                1400,
                lambda: self._copy_button.setText("Adresse kopieren"),
            )
