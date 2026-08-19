from __future__ import annotations

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QDoubleValidator, QPainter, QPen, QPixmap
from PySide6.QtWidgets import (
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)


GREEN = "#2EAD4A"
ORANGE = "#F7931A"
TEXT = "#1E2025"
MUTED = "#6D7078"
BORDER = "#D8DBE0"
SURFACE = "#FFFFFF"
PURPLE = "#8E159D"


class SendPage(QWidget):
    send_requested = Signal(str, str)
    review_requested = Signal()
    sign_requested = Signal()
    broadcast_requested = Signal()
    reset_requested = Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("Page")

        self._mode = "prepare"
        self._step_nodes: list[QLabel] = []
        self._step_lines: list[QFrame] = []
        self._step_labels: list[QLabel] = []

        self._build_ui()
        self._set_progress(completed=0, current=0)

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
            QLabel#FormLabel {
                color: #1E2025;
                font-size: 15px;
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
            QLineEdit#Input:disabled {
                background: #F2F3F5;
                color: #7B7F87;
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
            QFrame#SendDivider {
                background: #D9DCE3;
                border: none;
                min-height: 1px;
                max-height: 1px;
            }
            QLabel#SendSecurityIcon,
            QLabel#SendSafetyIcon {
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

        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(6)

        title_label = QLabel("SENDEN")
        title_label.setObjectName("PageTitle")

        subtitle_label = QLabel(
            "Transaktion vorbereiten, auf der Hardware prüfen, signieren und anschließend senden."
        )
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge(), alignment=Qt.AlignTop)
        outer.addLayout(head)

        # Open content area – no enclosing card.
        content = QWidget()
        layout = QVBoxLayout(content)
        layout.setContentsMargins(0, 18, 0, 0)
        layout.setSpacing(16)

        section_head = QHBoxLayout()
        section_head.setSpacing(10)

        section_icon = QLabel("↗")
        section_icon.setObjectName("SectionIcon")
        section_icon.setFixedWidth(18)

        title = QLabel("BC2 senden")
        title.setObjectName("SectionTitle")

        section_head.addWidget(section_icon)
        section_head.addWidget(title)
        section_head.addStretch()
        layout.addLayout(section_head)

        layout.addWidget(self._build_progress())
        layout.addSpacing(6)

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

        # Preview/result becomes its own open section.
        self._preview = QFrame()
        self._preview.setObjectName("ResultPanel")
        self._preview.setVisible(False)

        preview_layout = QVBoxLayout(self._preview)
        preview_layout.setContentsMargins(0, 0, 0, 0)
        preview_layout.setSpacing(8)

        preview_title_row = QHBoxLayout()
        preview_title_row.setSpacing(10)

        preview_icon = QLabel("≡")
        preview_icon.setObjectName("SectionIcon")
        preview_icon.setFixedWidth(18)

        preview_title = QLabel("Transaktionsentwurf")
        preview_title.setObjectName("CardTitle")

        preview_title_row.addWidget(preview_icon)
        preview_title_row.addWidget(preview_title)
        preview_title_row.addStretch()

        self._preview_text = QLabel("")
        self._preview_text.setObjectName("BodyText")
        self._preview_text.setWordWrap(True)
        self._preview_text.setTextInteractionFlags(Qt.TextSelectableByMouse)

        self._status = QLabel("")
        self._status.setObjectName("SmallMuted")
        self._status.setWordWrap(True)
        self._status.setTextInteractionFlags(Qt.TextSelectableByMouse)

        preview_layout.addLayout(preview_title_row)
        preview_layout.addWidget(self._preview_text)
        preview_layout.addWidget(self._status)

        preview_divider = QFrame()
        preview_divider.setObjectName("SendDivider")
        preview_divider.setFrameShape(QFrame.HLine)

        layout.addSpacing(8)
        layout.addWidget(preview_divider)
        layout.addSpacing(2)
        layout.addWidget(self._preview)

        actions = QHBoxLayout()

        self._change_button = QPushButton("Ändern")
        self._change_button.setObjectName("OutlineButton")
        self._change_button.setVisible(False)
        self._change_button.clicked.connect(self._reset)

        self._action_button = QPushButton("Weiter")
        self._action_button.setObjectName("PrimaryButton")
        self._action_button.clicked.connect(self._run_action)

        actions.addWidget(self._change_button)
        actions.addStretch()
        actions.addWidget(self._action_button)

        layout.addLayout(actions)
        layout.addStretch()

        outer.addWidget(content, 1)

        footer_divider = QFrame()
        footer_divider.setObjectName("SendDivider")
        footer_divider.setFrameShape(QFrame.HLine)
        outer.addWidget(footer_divider)
        outer.addWidget(self._safety_banner())

    def _build_progress(self) -> QWidget:
        container = QWidget()
        layout = QHBoxLayout(container)
        layout.setContentsMargins(0, 4, 0, 8)
        layout.setSpacing(0)

        labels = ("Entwurf", "Prüfen", "Signieren", "Senden")

        for index, label_text in enumerate(labels):
            # Each node and its label share the same vertical column.
            # This keeps the text exactly centered below the corresponding point.
            step = QWidget()
            step_layout = QVBoxLayout(step)
            step_layout.setContentsMargins(0, 0, 0, 0)
            step_layout.setSpacing(7)

            node = QLabel(str(index + 1))
            node.setAlignment(Qt.AlignCenter)
            node.setFixedSize(30, 30)

            label = QLabel(label_text)
            label.setAlignment(Qt.AlignHCenter | Qt.AlignTop)
            label.setFixedHeight(20)

            self._step_nodes.append(node)
            self._step_labels.append(label)

            node_row = QHBoxLayout()
            node_row.setContentsMargins(0, 0, 0, 0)
            node_row.addStretch()
            node_row.addWidget(node)
            node_row.addStretch()

            step_layout.addLayout(node_row)
            step_layout.addWidget(label)
            layout.addWidget(step)

            if index < len(labels) - 1:
                line_holder = QWidget()
                line_layout = QVBoxLayout(line_holder)
                line_layout.setContentsMargins(0, 14, 0, 0)
                line_layout.setSpacing(0)

                line = QFrame()
                line.setFixedHeight(2)
                line.setMinimumWidth(65)

                self._step_lines.append(line)
                line_layout.addWidget(line)
                line_layout.addStretch()

                layout.addWidget(line_holder, 1)

        return container

    def _set_progress(self, completed: int, current: int | None) -> None:
        for index, node in enumerate(self._step_nodes):
            if index < completed:
                node.setText("✓")
                node.setStyleSheet(
                    f"background:{GREEN}; color:white; border:none;"
                    "border-radius:15px; font-weight:600; font-size:13px;"
                )
            elif current is not None and index == current:
                node.setText(str(index + 1))
                node.setStyleSheet(
                    f"background:{PURPLE}; color:white; border:none;"
                    "border-radius:15px; font-weight:600; font-size:13px;"
                )
            else:
                node.setText(str(index + 1))
                node.setStyleSheet(
                    f"background:{SURFACE}; color:{MUTED};"
                    f"border:2px solid {BORDER}; border-radius:15px;"
                    "font-weight:600; font-size:13px;"
                )

        for index, line in enumerate(self._step_lines):
            line.setStyleSheet(
                f"background:{GREEN if index < completed else BORDER};"
                "border:none;"
            )

        for index, label in enumerate(self._step_labels):
            if index < completed:
                color = GREEN
                weight = 600
            elif current is not None and index == current:
                color = PURPLE
                weight = 600
            else:
                color = MUTED
                weight = 500

            label.setStyleSheet(
                f"color:{color}; font-size:13px; font-weight:{weight};"
            )

    def _run_action(self) -> None:
        if self._mode == "prepare":
            self._submit()
        elif self._mode == "review":
            self.review_requested.emit()
        elif self._mode == "sign":
            self.sign_requested.emit()
        elif self._mode == "broadcast":
            self.broadcast_requested.emit()

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

        if not amount_text:
            self._error.setText("Bitte einen Betrag eingeben.")
            return

        self.send_requested.emit(address, amount_text)

    def _reset(self) -> None:
        self._mode = "prepare"
        self._error.setText("")
        self._preview.setVisible(False)
        self._status.setText("")
        self._address_input.setEnabled(True)
        self._amount_input.setEnabled(True)
        self._change_button.setVisible(False)
        self._action_button.setEnabled(True)
        self._action_button.setText("Weiter")
        self._set_progress(completed=0, current=0)
        self.reset_requested.emit()

    def show_hardware_required(self) -> None:
        self.show_prepare_failed(
            "Hardware Wallet nicht verbunden. "
            "Senden ist ohne Hardware-Bestätigung nicht möglich."
        )

    def show_preparing(self) -> None:
        self._error.setText("")
        self._action_button.setEnabled(False)
        self._action_button.setText("Entwurf wird berechnet …")

    def show_plan(self, plan) -> None:
        self._mode = "review"
        self._error.setText("")
        self._address_input.setEnabled(False)
        self._amount_input.setEnabled(False)
        self._change_button.setVisible(True)

        self._preview_text.setText(
            f"Empfänger: {plan.recipient}\n"
            f"Betrag: {plan.amount / 100_000_000:.8f} BC2\n"
            f"Netzwerkgebühr: {plan.fee / 100_000_000:.8f} BC2 "
            f"({plan.fee_rate} sat/vB)\n"
            f"Wechselgeld: {plan.change / 100_000_000:.8f} BC2\n"
            f"Eingänge: {plan.input_count} · geschätzt {plan.estimated_vbytes} vB"
        )

        self._status.setText(
            "Entwurf erstellt. Als Nächstes auf der Hardware prüfen."
        )
        self._preview.setVisible(True)

        self._action_button.setEnabled(True)
        self._action_button.setText("Auf Hardware prüfen")
        self._set_progress(completed=1, current=1)

    def show_review_started(self) -> None:
        self._error.setText("")
        self._action_button.setEnabled(False)
        self._action_button.setText("Warte auf Hardware …")
        self._status.setText(
            "PIN auf der Hardware eingeben und Transaktion dort prüfen."
        )

    def show_review_progress(self, message: str) -> None:
        self._status.setText(message)

    def show_review_result(self, approved: bool) -> None:
        if approved:
            self._mode = "sign"
            self._action_button.setEnabled(True)
            self._action_button.setText("Auf Hardware signieren")
            self._status.setText(
                "✓ Auf der Hardware bestätigt. "
                "Als Nächstes wird die bestätigte Transaktion signiert."
            )
            self._set_progress(completed=2, current=2)
        else:
            self._mode = "review"
            self._action_button.setEnabled(True)
            self._action_button.setText("Erneut auf Hardware prüfen")
            self._status.setText("Transaktion wurde auf der Hardware abgelehnt.")
            self._set_progress(completed=1, current=1)

    def show_review_failed(self, message: str) -> None:
        self._mode = "review"
        self._action_button.setEnabled(True)
        self._action_button.setText("Auf Hardware prüfen")
        self._error.setText(message)
        self._set_progress(completed=1, current=1)

    def show_sign_started(self) -> None:
        self._error.setText("")
        self._action_button.setEnabled(False)
        self._action_button.setText("Hardware signiert …")
        self._status.setText(
            "Die privaten Schlüssel bleiben ausschließlich auf der Hardware."
        )

    def show_sign_progress(self, message: str) -> None:
        self._status.setText(message)

    def show_signed(self, signed) -> None:
        self._mode = "broadcast"
        self._action_button.setEnabled(True)
        self._action_button.setText("Transaktion senden")
        self._status.setText(
            f"✓ Auf Hardware signiert.\n"
            f"TXID: {signed.txid}\n"
            "Die Transaktion wurde noch nicht ins Netzwerk gesendet."
        )
        self._set_progress(completed=3, current=3)

    def show_sign_failed(self, message: str) -> None:
        self._mode = "sign"
        self._action_button.setEnabled(True)
        self._action_button.setText("Auf Hardware signieren")
        self._error.setText(message)
        self._set_progress(completed=2, current=2)

    def show_broadcast_started(self) -> None:
        self._error.setText("")
        self._action_button.setEnabled(False)
        self._action_button.setText("Wird gesendet …")
        self._status.setText(
            "Die signierte Transaktion wird an den BC2 Electrum-Server übertragen."
        )

    def show_broadcast_success(self, txid: str) -> None:
        self._mode = "done"
        self._action_button.setEnabled(False)
        self._action_button.setText("✓ Gesendet")
        self._change_button.setVisible(False)
        self._status.setText(
            f"✓ Transaktion erfolgreich ins Netzwerk gesendet.\n"
            f"TXID: {txid}"
        )
        self._set_progress(completed=4, current=None)

    def show_broadcast_failed(self, message: str) -> None:
        self._mode = "broadcast"
        self._action_button.setEnabled(True)
        self._action_button.setText("Erneut senden")
        self._error.setText(message)
        self._set_progress(completed=3, current=3)

    def show_prepare_failed(self, message: str) -> None:
        self._mode = "prepare"
        self._action_button.setEnabled(True)
        self._action_button.setText("Weiter")
        self._error.setText(message)
        self._set_progress(completed=0, current=0)

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
        row.setContentsMargins(0, 2, 0, 2)
        row.setSpacing(11)

        icon = QLabel()
        icon.setObjectName("SendSecurityIcon")
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
        icon.setObjectName("SendSafetyIcon")
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

