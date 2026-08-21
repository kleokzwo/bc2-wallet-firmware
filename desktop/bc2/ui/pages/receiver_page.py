from __future__ import annotations

from io import BytesIO

import qrcode

from PySide6.QtCore import Qt, Signal, QTimer
from PySide6.QtGui import QGuiApplication, QPainter, QPen, QPixmap
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
        outer.setSpacing(24)

        self.setStyleSheet("""
            QLabel#PageTitle {
                color: #8E159D;
                font-size: 30px;
                font-weight: 500;
            }
            QLabel#PageSubtitle {
                color: #626775;
                font-size: 15px;
                font-weight: 400;
            }
            QLabel#SectionIcon {
                color: #8E159D;
                font-size: 22px;
                font-weight: 500;
                background: transparent;
                border: none;
            }
            QLabel#SectionTitle {
                color: #8E159D;
                font-size: 20px;
                font-weight: 600;
            }
            QLabel#BodyText {
                color: #626775;
                font-size: 15px;
                font-weight: 400;
            }
            QLabel#CardTitle {
                color: #8E159D;
                font-size: 18px;
                font-weight: 600;
            }
            QLabel#SmallMuted {
                color: #626775;
                font-size: 14px;
                font-weight: 400;
            }
            QPushButton#PrimaryButton {
                background: #8E159D;
                color: white;
                border: none;
                border-radius: 10px;
                padding: 11px 18px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton#PrimaryButton:hover {
                background: #781185;
            }
            QPushButton#PrimaryButton:pressed {
                background: #691073;
            }
            QPushButton#PrimaryButton:disabled {
                background: #D8D8DC;
                color: white;
            }
            QPushButton#OutlineButton {
                background: transparent;
                color: #8E159D;
                border: 1px solid #8E159D;
                border-radius: 10px;
                padding: 10px 16px;
                font-size: 15px;
                font-weight: 600;
            }
            QPushButton#OutlineButton:hover {
                background: rgba(142, 21, 157, 0.06);
            }
            QFrame#SecurityBadge,
            QFrame#SafetyBanner,
            QFrame#ResultPanel {
                background: transparent;
                border: none;
            }
            QFrame#ReceiveDivider {
                background: #D9DCE3;
                border: none;
                min-height: 1px;
                max-height: 1px;
            }
            QLabel#ReceiveSecurityIcon,
            QLabel#ReceiveSafetyIcon {
                background: transparent;
                border: none;
            }
            QLabel#SecurityPrimary {
                color: #8E159D;
                font-size: 15px;
                font-weight: 600;
            }
            QLabel#SecuritySecondary {
                color: #626775;
                font-size: 13px;
                font-weight: 400;
            }
            QLabel#SafetyTitle {
                color: #8E159D;
                font-size: 15px;
                font-weight: 600;
            }
            QLabel#SafetyText {
                color: #626775;
                font-size: 14px;
                font-weight: 400;
            }
        """)

        # Page header
        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(6)

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
        head.addWidget(self._security_badge(), alignment=Qt.AlignTop)
        outer.addLayout(head)

        # Open content, aligned with the page header.
        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setContentsMargins(0, 18, 0, 0)
        layout.setSpacing(18)

        section_head = QHBoxLayout()
        section_head.setSpacing(10)

        section_icon = QLabel("↓")
        section_icon.setObjectName("SectionIcon")
        section_icon.setFixedWidth(18)

        title = QLabel("BC2 empfangen")
        title.setObjectName("SectionTitle")

        section_head.addWidget(section_icon)
        section_head.addWidget(title)
        section_head.addStretch()

        self._state = QLabel(
            "Verbinde deine Hardware Wallet, um eine Empfangsadresse anzufordern."
        )
        self._state.setObjectName("BodyText")
        self._state.setWordWrap(True)

        self._request_button = QPushButton("Empfangsadresse anfordern")
        self._request_button.setObjectName("PrimaryButton")
        self._request_button.clicked.connect(self.request_receive_requested)

        layout.addLayout(section_head)
        layout.addWidget(self._state)
        layout.addWidget(self._request_button, alignment=Qt.AlignLeft)

        # Divider between the action section and the result section.
        divider = QFrame()
        divider.setObjectName("ReceiveDivider")
        divider.setFrameShape(QFrame.HLine)
        layout.addSpacing(18)
        layout.addWidget(divider)
        layout.addSpacing(2)

        # Result area – second distinct section, still without a box/card.
        self._result = QFrame()
        self._result.setObjectName("ResultPanel")
        rr = QVBoxLayout(self._result)
        rr.setContentsMargins(0, 0, 0, 0)
        rr.setSpacing(10)

        result_head = QHBoxLayout()
        result_head.setSpacing(10)

        result_icon = QLabel("⌁")
        result_icon.setObjectName("SectionIcon")
        result_icon.setFixedWidth(18)

        self._result_title = QLabel("Noch keine Adresse")
        self._result_title.setObjectName("CardTitle")

        result_head.addWidget(result_icon)
        result_head.addWidget(self._result_title)
        result_head.addStretch()

        self._result_text = QLabel(
            "Sobald du die Adresse auf der Hardware bestätigt hast, "
            "erscheint sie hier zusammen mit QR-Code und Kopierfunktion."
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

        rr.addLayout(result_head)
        rr.addWidget(self._qr, alignment=Qt.AlignLeft)
        rr.addWidget(self._result_text)
        rr.addWidget(self._copy_button, alignment=Qt.AlignLeft)

        layout.addWidget(self._result)
        layout.addStretch()

        outer.addWidget(content, 1)

        # Divider above the safety footer, matching the Dashboard language.
        footer_divider = QFrame()
        footer_divider.setObjectName("ReceiveDivider")
        footer_divider.setFrameShape(QFrame.HLine)
        outer.addWidget(footer_divider)
        outer.addWidget(self._safety_banner())

    def _security_badge(self) -> QWidget:
        frame = QFrame()
        frame.setObjectName("SecurityBadge")
        frame.setFixedWidth(250)

        row = QHBoxLayout(frame)
        row.setContentsMargins(0, 2, 0, 2)
        row.setSpacing(11)

        icon = QLabel()
        icon.setObjectName("ReceiveSecurityIcon")
        icon.setAlignment(Qt.AlignCenter)
        icon.setFixedSize(34, 34)
        icon.setPixmap(self._hat_glasses_icon(30, "#23B55A"))

        text = QVBoxLayout()
        text.setSpacing(1)

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
        row.setContentsMargins(0, 4, 0, 0)
        row.setSpacing(12)

        icon = QLabel()
        icon.setObjectName("ReceiveSafetyIcon")
        icon.setAlignment(Qt.AlignCenter)
        icon.setFixedSize(34, 34)
        icon.setPixmap(self._user_key_icon(30, "#8E159D"))

        text = QVBoxLayout()
        text.setSpacing(2)

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

    def _hat_glasses_icon(self, size: int, color: str) -> QPixmap:
        pixmap = QPixmap(size, size)
        pixmap.fill(Qt.transparent)

        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing)
        pen = QPen(color)
        pen.setWidthF(max(1.8, size / 14))
        pen.setCapStyle(Qt.RoundCap)
        pen.setJoinStyle(Qt.RoundJoin)
        painter.setPen(pen)

        # Hat
        painter.drawLine(int(size * 0.18), int(size * 0.38), int(size * 0.82), int(size * 0.38))
        painter.drawLine(int(size * 0.32), int(size * 0.34), int(size * 0.40), int(size * 0.20))
        painter.drawLine(int(size * 0.40), int(size * 0.20), int(size * 0.60), int(size * 0.20))
        painter.drawLine(int(size * 0.60), int(size * 0.20), int(size * 0.68), int(size * 0.34))

        # Glasses
        painter.drawEllipse(
            int(size * 0.20), int(size * 0.50),
            int(size * 0.25), int(size * 0.25)
        )
        painter.drawEllipse(
            int(size * 0.55), int(size * 0.50),
            int(size * 0.25), int(size * 0.25)
        )
        painter.drawLine(int(size * 0.45), int(size * 0.61), int(size * 0.55), int(size * 0.61))

        painter.end()
        return pixmap

    def _user_key_icon(self, size: int, color: str) -> QPixmap:
        pixmap = QPixmap(size, size)
        pixmap.fill(Qt.transparent)

        painter = QPainter(pixmap)
        painter.setRenderHint(QPainter.Antialiasing)
        pen = QPen(color)
        pen.setWidthF(max(1.8, size / 14))
        pen.setCapStyle(Qt.RoundCap)
        pen.setJoinStyle(Qt.RoundJoin)
        painter.setPen(pen)

        # User head and shoulders
        painter.drawEllipse(
            int(size * 0.18), int(size * 0.10),
            int(size * 0.30), int(size * 0.30)
        )
        painter.drawArc(
            int(size * 0.08), int(size * 0.38),
            int(size * 0.52), int(size * 0.44),
            20 * 16, 140 * 16
        )

        # Key
        painter.drawEllipse(
            int(size * 0.58), int(size * 0.52),
            int(size * 0.18), int(size * 0.18)
        )
        painter.drawLine(int(size * 0.74), int(size * 0.61), int(size * 0.92), int(size * 0.61))
        painter.drawLine(int(size * 0.85), int(size * 0.61), int(size * 0.85), int(size * 0.72))
        painter.drawLine(int(size * 0.92), int(size * 0.61), int(size * 0.92), int(size * 0.68))

        painter.end()
        return pixmap

    def reset_wallet_view(self) -> None:
        """Remove every wallet-specific receive value from the visible page."""
        self._set_qr(None)
        self._result_title.setText("Noch keine Adresse")
        self._result_text.setText(
            "Sobald du die Adresse auf der Hardware bestätigt hast, "
            "erscheint sie hier zusammen mit QR-Code und Kopierfunktion."
        )
        self._result_text.setTextInteractionFlags(Qt.NoTextInteraction)
        self._copy_button.setVisible(False)
        self._copy_button.setText("Adresse kopieren")
        self._state.setText(
            "Entsperre deine Hardware Wallet, um eine Empfangsadresse anzufordern."
        )

    def show_cached_address(self, address: str) -> None:
        """Show a public address only from the currently authenticated wallet cache."""
        value = str(address).strip()
        if not value:
            self.reset_wallet_view()
            return
        self._result_title.setText("Letzte bekannte Empfangsadresse")
        self._set_qr(value)
        self._result_text.setText(f"Adresse: {value}")
        self._result_text.setTextInteractionFlags(Qt.TextSelectableByMouse)
        self._copy_button.setVisible(True)

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
