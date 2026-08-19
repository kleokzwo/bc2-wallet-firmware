from __future__ import annotations

from PySide6.QtCore import Qt
from PySide6.QtGui import QPainter, QPen, QPixmap
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
        outer.setSpacing(24)

        # UI-only styling. The original centered About layout is preserved.
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
            QLabel#AboutTitle {
                color: #8E159D;
                font-size: 24px;
                font-weight: 600;
            }
            QLabel#BodyText {
                color: #626775;
                font-size: 15px;
                font-weight: 400;
            }
            QLabel#SmallMuted {
                color: #626775;
                font-size: 14px;
                font-weight: 400;
            }
            QFrame#SecurityBadge,
            QFrame#SafetyBanner {
                background: transparent;
                border: none;
            }
            QFrame#AboutDivider {
                background: #D9DCE3;
                border: none;
                min-height: 1px;
                max-height: 1px;
            }
            QLabel#AboutSecurityIcon,
            QLabel#AboutSafetyIcon {
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
        head.addWidget(self._security_badge(), alignment=Qt.AlignTop)
        outer.addLayout(head)

        # Original centered About content, only without the surrounding box.
        content = QWidget()
        layout = QVBoxLayout(content)
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

        outer.addWidget(content, 1)

        # Same footer treatment as Receive/Send/Settings.
        divider = QFrame()
        divider.setObjectName("AboutDivider")
        divider.setFrameShape(QFrame.HLine)
        outer.addWidget(divider)
        outer.addWidget(self._safety_banner())

    def _security_badge(self) -> QWidget:
        frame = QFrame()
        frame.setObjectName("SecurityBadge")
        frame.setFixedWidth(250)

        row = QHBoxLayout(frame)
        row.setContentsMargins(0, 2, 0, 2)
        row.setSpacing(11)

        icon = QLabel()
        icon.setObjectName("AboutSecurityIcon")
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
        icon.setObjectName("AboutSafetyIcon")
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

        painter.drawEllipse(int(size * 0.20), int(size * 0.50), int(size * 0.25), int(size * 0.25))
        painter.drawEllipse(int(size * 0.55), int(size * 0.50), int(size * 0.25), int(size * 0.25))
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

        painter.drawEllipse(int(size * 0.18), int(size * 0.10), int(size * 0.30), int(size * 0.30))
        painter.drawArc(
            int(size * 0.08), int(size * 0.38),
            int(size * 0.52), int(size * 0.44),
            20 * 16, 140 * 16
        )

        painter.drawEllipse(int(size * 0.58), int(size * 0.52), int(size * 0.18), int(size * 0.18))
        painter.drawLine(int(size * 0.74), int(size * 0.61), int(size * 0.92), int(size * 0.61))
        painter.drawLine(int(size * 0.85), int(size * 0.61), int(size * 0.85), int(size * 0.72))
        painter.drawLine(int(size * 0.92), int(size * 0.61), int(size * 0.92), int(size * 0.68))

        painter.end()
        return pixmap
