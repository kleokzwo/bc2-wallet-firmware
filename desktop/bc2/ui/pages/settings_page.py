from __future__ import annotations

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QPainter, QPen, QPixmap
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class SettingsPage(QWidget):
    save_requested = Signal(str, str)

    def __init__(self, electrum_server: str, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._electrum_server = electrum_server
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(24)

        # Local UI styling only.
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
            QLabel#FormLabel {
                color: #1E2025;
                font-size: 15px;
                font-weight: 600;
            }
            QLabel#SmallMuted {
                color: #626775;
                font-size: 14px;
                font-weight: 400;
            }
            QLabel#SuccessText {
                color: #23B55A;
                font-size: 14px;
                font-weight: 600;
            }
            QLabel#ErrorText {
                color: #C43D3D;
                font-size: 14px;
                font-weight: 500;
            }
            QLineEdit#Input {
                background: white;
                color: #1E2025;
                border: 1px solid #C9CDD5;
                border-radius: 10px;
                padding: 11px 14px;
                font-size: 15px;
                font-weight: 400;
                min-height: 24px;
            }
            QLineEdit#Input:focus {
                border: 1px solid #8E159D;
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
            QFrame#SecurityBadge,
            QFrame#SafetyBanner {
                background: transparent;
                border: none;
            }
            QFrame#SettingsDivider {
                background: #D9DCE3;
                border: none;
                min-height: 1px;
                max-height: 1px;
            }
            QLabel#SettingsSecurityIcon,
            QLabel#SettingsSafetyIcon {
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

        title_label = QLabel("EINSTELLUNGEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Nur Einstellungen, die wirklich nötig sind."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge(), alignment=Qt.AlignTop)
        outer.addLayout(head)

        # Open settings content – no surrounding card.
        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setContentsMargins(0, 18, 0, 0)
        layout.setSpacing(14)

        network_head = QHBoxLayout()
        network_head.setSpacing(10)

        network_icon = QLabel("◎")
        network_icon.setObjectName("SectionIcon")
        network_icon.setFixedWidth(18)

        title = QLabel("Netzwerk")
        title.setObjectName("SectionTitle")

        network_head.addWidget(network_icon)
        network_head.addWidget(title)
        network_head.addStretch()
        layout.addLayout(network_head)

        layout.addWidget(self._form_label("Electrum Server"))

        self._server_input = QLineEdit(self._electrum_server)
        self._server_input.setObjectName("Input")
        self._server_input.setPlaceholderText("host:port")
        layout.addWidget(self._server_input)

        note = QLabel(
            "SSL wird für den BC2 Electrum-Zugriff verwendet. "
            "Bestätigte Empfangsadressen werden automatisch synchronisiert."
        )
        note.setObjectName("SmallMuted")
        note.setWordWrap(True)
        layout.addWidget(note)

        # Separate second settings section.
        divider = QFrame()
        divider.setObjectName("SettingsDivider")
        divider.setFrameShape(QFrame.HLine)

        layout.addSpacing(18)
        layout.addWidget(divider)
        layout.addSpacing(2)

        sync_head = QHBoxLayout()
        sync_head.setSpacing(10)

        sync_icon = QLabel("↻")
        sync_icon.setObjectName("SectionIcon")
        sync_icon.setFixedWidth(18)

        sync_title = QLabel("Empfangsadresse synchronisieren")
        sync_title.setObjectName("SectionTitle")

        sync_head.addWidget(sync_icon)
        sync_head.addWidget(sync_title)
        sync_head.addStretch()
        layout.addLayout(sync_head)

        layout.addWidget(
            self._form_label(
                "Vorhandene Empfangsadresse für Sync hinzufügen (optional)"
            )
        )

        self._sync_address_input = QLineEdit()
        self._sync_address_input.setObjectName("Input")
        self._sync_address_input.setPlaceholderText("bc1q…")
        layout.addWidget(self._sync_address_input)

        old_note = QLabel(
            "Nur für Adressen nötig, die vor v0.42.0 erzeugt wurden. "
            "Neue Empfangsadressen werden automatisch gespeichert."
        )
        old_note.setObjectName("SmallMuted")
        old_note.setWordWrap(True)
        layout.addWidget(old_note)

        self._save_button = QPushButton("Einstellungen speichern")
        self._save_button.setObjectName("PrimaryButton")
        self._save_button.clicked.connect(self._submit)
        layout.addWidget(self._save_button, alignment=Qt.AlignLeft)

        self._message = QLabel("")
        self._message.setObjectName("SuccessText")
        layout.addWidget(self._message)

        layout.addStretch()
        outer.addWidget(content, 1)

        # Footer separator and safety note.
        footer_divider = QFrame()
        footer_divider.setObjectName("SettingsDivider")
        footer_divider.setFrameShape(QFrame.HLine)
        outer.addWidget(footer_divider)
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
        row.setContentsMargins(0, 2, 0, 2)
        row.setSpacing(11)

        icon = QLabel()
        icon.setObjectName("SettingsSecurityIcon")
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
        icon.setObjectName("SettingsSafetyIcon")
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

        painter.drawLine(int(size * 0.18), int(size * 0.38), int(size * 0.82), int(size * 0.38))
        painter.drawLine(int(size * 0.32), int(size * 0.34), int(size * 0.40), int(size * 0.20))
        painter.drawLine(int(size * 0.40), int(size * 0.20), int(size * 0.60), int(size * 0.20))
        painter.drawLine(int(size * 0.60), int(size * 0.20), int(size * 0.68), int(size * 0.34))

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

        painter.drawEllipse(
            int(size * 0.18), int(size * 0.10),
            int(size * 0.30), int(size * 0.30)
        )
        painter.drawArc(
            int(size * 0.08), int(size * 0.38),
            int(size * 0.52), int(size * 0.44),
            20 * 16, 140 * 16
        )

        painter.drawEllipse(
            int(size * 0.58), int(size * 0.52),
            int(size * 0.18), int(size * 0.18)
        )
        painter.drawLine(int(size * 0.74), int(size * 0.61), int(size * 0.92), int(size * 0.61))
        painter.drawLine(int(size * 0.85), int(size * 0.61), int(size * 0.85), int(size * 0.72))
        painter.drawLine(int(size * 0.92), int(size * 0.61), int(size * 0.92), int(size * 0.68))

        painter.end()
        return pixmap

    def _submit(self) -> None:
        value = self._server_input.text().strip()

        if ":" not in value or value.startswith(":") or value.endswith(":"):
            self.show_error(
                "Bitte einen Server im Format host:port eingeben."
            )
            return

        host, port = value.rsplit(":", 1)

        if (
            not host.strip()
            or not port.isdigit()
            or not (1 <= int(port) <= 65535)
        ):
            self.show_error(
                "Bitte einen gültigen Host und Port eingeben."
            )
            return

        address = self._sync_address_input.text().strip()
        self.save_requested.emit(value, address)

    def show_error(self, message: str) -> None:
        self._message.setObjectName("ErrorText")
        self._message.setText(message)
        self._repolish(self._message)

    def show_saved(self) -> None:
        self._message.setObjectName("SuccessText")
        self._message.setText("✓ Gespeichert")
        self._sync_address_input.clear()
        self._repolish(self._message)

    def set_server(self, value: str) -> None:
        self._server_input.setText(value)

    @staticmethod
    def _repolish(widget: QWidget) -> None:
        widget.style().unpolish(widget)
        widget.style().polish(widget)
