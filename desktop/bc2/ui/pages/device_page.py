from __future__ import annotations

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QPixmap
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


class DevicePage(QWidget):
    scan_requested = Signal()

    def __init__(self, asset_path, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._asset_path = asset_path
        self._values: dict[str, QLabel] = {}
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        # Page header
        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel("HARDWARE WALLET")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Verbindung und Eigenschaften deiner BC2 Hardware Wallet."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())

        outer.addLayout(head)

        # Device content
        card = QFrame()
        card.setObjectName("Card")

        layout = QVBoxLayout(card)
        layout.setContentsMargins(26, 24, 26, 24)
        layout.setSpacing(20)

        header = QHBoxLayout()

        self._status_icon = QLabel("…")
        self._status_icon.setAlignment(Qt.AlignCenter)
        self._status_icon.setFixedSize(40, 40)
        self._status_icon.setObjectName("StatusIconScanning")

        hs = QVBoxLayout()

        self._status = QLabel("Suche nach BC2 Hardware Wallet …")
        self._status.setObjectName("SectionTitle")

        self._detail = QLabel("Serielle Geräte werden geprüft.")
        self._detail.setObjectName("SmallMuted")

        hs.addWidget(self._status)
        hs.addWidget(self._detail)

        self._badge = QLabel("Suche …")
        self._badge.setObjectName("ConnectionBadgeScanning")
        self._badge.setAlignment(Qt.AlignCenter)
        self._badge.setFixedSize(150, 36)

        header.addWidget(self._status_icon)
        header.addLayout(hs, 1)
        header.addWidget(self._badge)

        layout.addLayout(header)

        body = QHBoxLayout()
        body.setSpacing(28)

        visual = QFrame()
        visual.setObjectName("DeviceVisual")
        visual.setFixedSize(220, 285)

        vl = QVBoxLayout(visual)
        vl.setContentsMargins(18, 18, 18, 18)

        self._photo = QLabel()
        self._photo.setAlignment(Qt.AlignCenter)

        pix = QPixmap(str(self._asset_path("bc2-device.png")))
        if not pix.isNull():
            self._photo.setPixmap(
                pix.scaled(
                    185,
                    245,
                    Qt.KeepAspectRatio,
                    Qt.SmoothTransformation,
                )
            )

        vl.addWidget(self._photo, 1)

        details = QFrame()
        details.setObjectName("DetailsPanel")

        dl = QVBoxLayout(details)
        dl.setContentsMargins(0, 0, 0, 0)
        dl.setSpacing(0)

        for key in (
            "Gerät",
            "Hardware",
            "Display",
            "Revision",
            "Port",
            "Gerätestatus",
            "Fähigkeiten",
            "Board-Revision",
        ):
            dl.addWidget(self._detail_row(key))

        body.addWidget(visual)
        body.addWidget(details, 1)

        layout.addLayout(body, 1)

        self._scan_button = QPushButton("↻   Erneut suchen")
        self._scan_button.setObjectName("PrimaryButton")
        self._scan_button.clicked.connect(self.scan_requested)
        layout.addWidget(self._scan_button, alignment=Qt.AlignHCenter)

        self._factory_reset_button = QPushButton(
            "Gerät zurücksetzen (nach Stabilisierung)"
        )
        self._factory_reset_button.setObjectName("DangerButton")
        self._factory_reset_button.setEnabled(False)
        layout.addWidget(
            self._factory_reset_button,
            alignment=Qt.AlignHCenter,
        )

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())

    def _detail_row(self, key: str) -> QWidget:
        row = QFrame()
        row.setObjectName("DetailRow")
        row.setMinimumHeight(42)

        lay = QHBoxLayout(row)
        lay.setContentsMargins(8, 0, 8, 0)

        label = QLabel(key)
        label.setObjectName("DetailLabel")
        label.setFixedWidth(145)

        value = QLabel("—")
        value.setObjectName("DetailValue")
        value.setTextInteractionFlags(Qt.TextSelectableByMouse)
        value.setWordWrap(True)

        self._values[key] = value

        lay.addWidget(label)
        lay.addWidget(value, 1)

        return row

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

    def show_scanning(self) -> None:
        self._scan_button.setEnabled(False)
        self._scan_button.setText("Suche …")

        self._status.setText("Suche nach BC2 Hardware Wallet …")
        self._detail.setText("Serielle Geräte werden sicher geprüft.")

        self._status_icon.setText("…")
        self._status_icon.setObjectName("StatusIconScanning")

        self._badge.setText("Suche …")
        self._badge.setObjectName("ConnectionBadgeScanning")

        self._repolish_status()

    def show_offline(self) -> None:
        self._scan_button.setEnabled(True)
        self._scan_button.setText("↻   Erneut suchen")
        self._factory_reset_button.setEnabled(False)

        self._status.setText("Hardware Wallet nicht verbunden")
        self._detail.setText(
            "Schließe die BC2 Hardware Wallet per USB an und suche erneut."
        )

        self._status_icon.setText("!")
        self._status_icon.setObjectName("StatusIconOffline")

        self._badge.setText("Nicht verbunden")
        self._badge.setObjectName("ConnectionBadgeOffline")

        for value in self._values.values():
            value.setText("—")

        self._repolish_status()

    def show_connected(self, device) -> None:
        self._scan_button.setEnabled(True)
        self._scan_button.setText("↻   Erneut suchen")
        self._factory_reset_button.setEnabled(False)

        self._status.setText("BC2 Hardware Wallet verbunden")
        self._detail.setText(
            "Das Gerät antwortet korrekt auf das BC2 USB-Protokoll."
        )

        self._status_icon.setText("✓")
        self._status_icon.setObjectName("StatusIconConnected")

        self._badge.setText("✓  Verbindung aktiv")
        self._badge.setObjectName("ConnectionBadgeConnected")

        values = {
            "Gerät": device.device_name,
            "Hardware": device.hardware_name,
            "Display": device.display_name,
            "Revision": device.firmware_revision,
            "Port": device.port,
            "Gerätestatus": (
                "unbekannt"
                if device.state is None
                else str(device.state)
            ),
            "Fähigkeiten": device.capabilities_text,
            "Board-Revision": str(device.board_revision),
        }

        for key, value in values.items():
            self._values[key].setText(value)

        self._repolish_status()

    def _repolish_status(self) -> None:
        for widget in (
            self._status_icon,
            self._badge,
        ):
            widget.style().unpolish(widget)
            widget.style().polish(widget)
