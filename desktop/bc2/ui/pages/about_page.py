from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtGui import QPixmap
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QVBoxLayout,
    QWidget,
)


class AboutPage(QWidget):
    def __init__(self, app_version: str, asset_path, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")
        self._app_version = app_version
        self._asset_path = asset_path
        self._build_ui()

    def _build_ui(self) -> None:
        outer = QVBoxLayout(self)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        # Page header
        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel("ÜBER")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Informationen über die BC2 Cold Wallet."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())

        outer.addLayout(head)

        # About content
        card = QFrame()
        card.setObjectName("Card")

        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(10)

        logo = QLabel()
        logo.setAlignment(Qt.AlignCenter)

        pix = QPixmap(str(self._asset_path("bc2-logo.png")))
        if not pix.isNull():
            logo.setPixmap(
                pix.scaled(
                    130,
                    130,
                    Qt.KeepAspectRatio,
                    Qt.SmoothTransformation,
                )
            )

        name = QLabel("BC2 Cold Wallet")
        name.setObjectName("AboutTitle")
        name.setAlignment(Qt.AlignCenter)

        version = QLabel(
            f"Desktop-Version {self._app_version}"
        )
        version.setObjectName("SmallMuted")
        version.setAlignment(Qt.AlignCenter)

        text = QLabel(
            "Eine einfache Cold-Wallet-Anwendung für Bitcoin II (BC2).\n\n"
            "Sicherheitsprinzip: Seed und private Schlüssel verlassen die Hardware niemals. "
            "Sicherheitskritische Aktionen werden auf dem Gerät geprüft und bestätigt."
        )
        text.setObjectName("BodyText")
        text.setAlignment(Qt.AlignCenter)
        text.setWordWrap(True)

        layout.addStretch()
        layout.addWidget(logo)
        layout.addWidget(name)
        layout.addWidget(version)
        layout.addSpacing(12)
        layout.addWidget(text)
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
