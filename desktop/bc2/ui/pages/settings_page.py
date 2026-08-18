from __future__ import annotations

from PySide6.QtCore import Qt, Signal
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
        outer.setSpacing(22)

        # Page header
        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

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
        head.addWidget(self._security_badge())

        outer.addLayout(head)

        # Settings content
        card = QFrame()
        card.setObjectName("Card")

        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(12)

        title = QLabel("Netzwerk")
        title.setObjectName("SectionTitle")
        layout.addWidget(title)

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

        layout.addSpacing(8)

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

        outer.addWidget(card, 1)

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
