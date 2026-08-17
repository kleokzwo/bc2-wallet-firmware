from __future__ import annotations

from pathlib import Path
from io import BytesIO

import qrcode

from PySide6.QtCore import QSettings, QTimer, Qt, Signal, Slot
from PySide6.QtGui import QDoubleValidator, QPixmap, QGuiApplication
from PySide6.QtWidgets import (
    QButtonGroup,
    QDialog,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QScrollArea,
    QSizePolicy,
    QStackedWidget,
    QVBoxLayout,
    QWidget,
)

from bc2.device.discovery import (
    DiscoveryResult, begin_create_wallet, begin_recovery, begin_unlock,
    submit_recovery_mnemonic, lock_wallet,
)
from bc2.device.model import DeviceInfo
from bc2.services.device_service import DeviceService
from bc2.services.receive_service import ReceiveService
from bc2.services.electrum_service import ElectrumService, BalanceResult, address_to_scriptpubkey
from bc2.ui.recovery_dialog import RecoveryDialog


APP_VERSION = "0.42.1"
DEFAULT_ELECTRUM = "infra1.bitcoin-ii.org:50009"

ORANGE = "#F7931A"
ORANGE_DARK = "#E77E00"
ORANGE_SOFT = "#FFF4E7"
GREEN = "#2EAD4A"
GREEN_SOFT = "#EAF8EE"
RED = "#C94B40"
RED_SOFT = "#FFF1EF"
BLUE_SOFT = "#EEF6FF"
TEXT = "#1E2025"
MUTED = "#6D7078"
BORDER = "#E2E4E8"
SURFACE = "#FFFFFF"
BACKGROUND = "#F5F6F8"
SIDEBAR = "#FBFBFC"
BALANCE = "#791597"


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BC2 Cold Wallet")
        self.resize(1240, 820)
        self.setMinimumSize(1040, 700)

        self._settings = QSettings("BC2", "ColdWallet")
        self._device: DeviceInfo | None = None
        self._setup_in_progress = False

        self._device_service = DeviceService(self)
        self._device_service.scan_started.connect(self._on_scan_started)
        self._device_service.scan_finished.connect(self._on_scan_finished)

        self._receive_service = ReceiveService(self)
        self._receive_service.started.connect(self._on_receive_started)
        self._receive_service.progress.connect(self._on_receive_progress)
        self._receive_service.finished.connect(self._on_receive_finished)
        self._receive_service.failed.connect(self._on_receive_failed)

        self._electrum_service = ElectrumService(self)
        self._electrum_service.started.connect(self._on_balance_sync_started)
        self._electrum_service.finished.connect(self._on_balance_sync_finished)
        self._electrum_service.failed.connect(self._on_balance_sync_failed)
        self._balance_timer = QTimer(self)
        self._balance_timer.setInterval(30000)
        self._balance_timer.timeout.connect(self._sync_balance)

        self._nav_buttons: dict[str, QPushButton] = {}
        self._pages: dict[str, QWidget] = {}

        self._build_ui()
        self._apply_styles()
        self._navigate("setup")
        self._sidebar.setVisible(False)
        QTimer.singleShot(300, self._device_service.scan)

    def _asset(self, name: str) -> Path:
        return Path(__file__).resolve().parents[2] / "assets" / name

    def _build_ui(self) -> None:
        root = QWidget()
        root.setObjectName("Root")
        row = QHBoxLayout(root)
        row.setContentsMargins(0, 0, 0, 0)
        row.setSpacing(0)

        self._sidebar = self._build_sidebar()
        row.addWidget(self._sidebar)
        row.addWidget(self._build_pages(), 1)
        self.setCentralWidget(root)

    def _build_sidebar(self) -> QWidget:
        sidebar = QFrame()
        sidebar.setObjectName("Sidebar")
        sidebar.setFixedWidth(250)
        layout = QVBoxLayout(sidebar)
        layout.setContentsMargins(26, 28, 20, 24)
        layout.setSpacing(7)

        brand = QHBoxLayout()
        brand.setSpacing(12)

        logo = QLabel()
        logo.setFixedSize(62, 62)
        pix = QPixmap(str(self._asset("bc2-logo.png")))
        if not pix.isNull():
            logo.setPixmap(pix.scaled(62, 62, Qt.KeepAspectRatio, Qt.SmoothTransformation))

        brand_text = QVBoxLayout()
        brand_text.setSpacing(0)
        name = QLabel("BC2")
        name.setObjectName("BrandName")
        sub = QLabel("COLD WALLET")
        sub.setObjectName("BrandSubtitle")
        brand_text.addWidget(name)
        brand_text.addWidget(sub)

        brand.addWidget(logo)
        brand.addLayout(brand_text)
        brand.addStretch()

        layout.addLayout(brand)
        layout.addSpacing(34)

        nav_items = [
            ("dashboard", "▦", "Dashboard"),
            ("receive", "↓", "Empfangen"),
            ("send", "↗", "Senden"),
            ("transactions", "☷", "Transaktionen"),
            ("device", "▣", "Gerät"),
            ("settings", "⚙", "Einstellungen"),
            ("about", "ⓘ", "Über"),
        ]

        group = QButtonGroup(self)
        group.setExclusive(True)

        for key, icon, label in nav_items:
            btn = QPushButton(f"{icon}    {label}")
            btn.setObjectName("NavButton")
            btn.setCheckable(True)
            btn.setCursor(Qt.PointingHandCursor)
            btn.setMinimumHeight(52)
            btn.clicked.connect(lambda checked=False, k=key: self._navigate(k))
            self._nav_buttons[key] = btn
            group.addButton(btn)
            layout.addWidget(btn)

        layout.addStretch()

        self._logout_button = QPushButton("⏻    Wallet sperren")
        self._logout_button.setObjectName("LogoutButton")
        self._logout_button.setCursor(Qt.PointingHandCursor)
        self._logout_button.setMinimumHeight(46)
        self._logout_button.clicked.connect(self._logout_wallet)
        layout.addWidget(self._logout_button)
        layout.addSpacing(10)

        self._sidebar_status = QFrame()
        self._sidebar_status.setObjectName("SidebarStatus")
        st = QVBoxLayout(self._sidebar_status)
        st.setContentsMargins(14, 11, 14, 11)
        st.setSpacing(4)
        self._sidebar_ready = QLabel("●  Suche Gerät …")
        self._sidebar_ready.setObjectName("SidebarSearching")
        version = QLabel(f"v{APP_VERSION}")
        version.setObjectName("SidebarVersion")
        st.addWidget(self._sidebar_ready)
        st.addWidget(version)
        layout.addWidget(self._sidebar_status)

        return sidebar

    def _build_pages(self) -> QWidget:
        host = QWidget()
        host.setObjectName("Content")
        layout = QVBoxLayout(host)
        layout.setContentsMargins(0, 0, 0, 0)

        self._stack = QStackedWidget()
        self._stack.setObjectName("PageStack")

        builders = {
            "setup": self._build_setup_page,
            "dashboard": self._build_dashboard_page,
            "receive": self._build_receive_page,
            "send": self._build_send_page,
            "transactions": self._build_transactions_page,
            "device": self._build_device_page,
            "settings": self._build_settings_page,
            "about": self._build_about_page,
        }

        for key, builder in builders.items():
            page = builder()
            self._pages[key] = page
            self._stack.addWidget(page)

        layout.addWidget(self._stack)
        return host

    def _page_shell(self, title: str, subtitle: str) -> tuple[QWidget, QVBoxLayout]:
        page = QWidget()
        page.setObjectName("Page")
        outer = QVBoxLayout(page)
        outer.setContentsMargins(42, 34, 42, 34)
        outer.setSpacing(22)

        head = QHBoxLayout()
        texts = QVBoxLayout()
        texts.setSpacing(5)

        title_label = QLabel(title)
        title_label.setObjectName("PageTitle")
        subtitle_label = QLabel(subtitle)
        subtitle_label.setObjectName("PageSubtitle")
        subtitle_label.setWordWrap(True)

        texts.addWidget(title_label)
        texts.addWidget(subtitle_label)
        head.addLayout(texts, 1)
        head.addWidget(self._security_badge())

        outer.addLayout(head)
        return page, outer

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
        a = QLabel("Sicher & Offline")
        a.setObjectName("SecurityPrimary")
        b = QLabel("Schlüssel bleiben auf Hardware")
        b.setObjectName("SecuritySecondary")
        text.addWidget(a)
        text.addWidget(b)
        row.addWidget(icon)
        row.addLayout(text, 1)
        return frame

    def _card(self) -> QFrame:
        frame = QFrame()
        frame.setObjectName("Card")
        return frame

    def _build_setup_page(self) -> QWidget:
        page = QWidget()
        page.setObjectName("Page")
        outer = QVBoxLayout(page)
        outer.setContentsMargins(90, 42, 90, 42)
        outer.setSpacing(16)
        outer.addStretch()

        logo = QLabel()
        logo.setAlignment(Qt.AlignCenter)
        pix = QPixmap(str(self._asset("bc2-logo.png")))
        if not pix.isNull():
            logo.setPixmap(pix.scaled(104, 104, Qt.KeepAspectRatio, Qt.SmoothTransformation))
        outer.addWidget(logo)

        title = QLabel("BC2 Hardware Wallet")
        title.setObjectName("SetupTitle")
        title.setAlignment(Qt.AlignCenter)
        outer.addWidget(title)

        intro = QLabel("Wallet entsperren, neu erstellen oder wiederherstellen.")
        intro.setObjectName("SetupText")
        intro.setAlignment(Qt.AlignCenter)
        intro.setWordWrap(True)
        outer.addWidget(intro)

        card = self._card()
        card.setMaximumWidth(500)
        card_layout = QVBoxLayout(card)
        card_layout.setContentsMargins(30, 24, 30, 24)
        card_layout.setSpacing(12)

        self._setup_status_title = QLabel("Hardware Wallet wird gesucht …")
        self._setup_status_title.setObjectName("SectionTitle")
        self._setup_status_title.setAlignment(Qt.AlignCenter)
        self._setup_status_text = QLabel("Verbinde deine BC2 Hardware Wallet per USB.")
        self._setup_status_text.setObjectName("BodyText")
        self._setup_status_text.setAlignment(Qt.AlignCenter)
        self._setup_status_text.setWordWrap(True)

        self._create_wallet_button = QPushButton("Create New Wallet")
        self._create_wallet_button.setObjectName("PrimaryButton")
        self._create_wallet_button.setCursor(Qt.PointingHandCursor)
        self._create_wallet_button.setMinimumWidth(300)
        self._create_wallet_button.setEnabled(False)
        self._create_wallet_button.clicked.connect(self._begin_wallet_creation)

        self._recovery_wallet_button = QPushButton("Recovery Wallet")
        self._recovery_wallet_button.setObjectName("RecoveryButton")
        self._recovery_wallet_button.setCursor(Qt.PointingHandCursor)
        self._recovery_wallet_button.setMinimumWidth(300)
        self._recovery_wallet_button.setEnabled(False)
        self._recovery_wallet_button.clicked.connect(self._begin_wallet_recovery)

        self._setup_scan_button = QPushButton("↻   Erneut suchen")
        self._setup_scan_button.setObjectName("OutlineButton")
        self._setup_scan_button.setCursor(Qt.PointingHandCursor)
        self._setup_scan_button.clicked.connect(self._device_service.scan)

        card_layout.addWidget(self._setup_status_title)
        card_layout.addWidget(self._setup_status_text)
        card_layout.addSpacing(10)
        card_layout.addWidget(self._create_wallet_button, alignment=Qt.AlignHCenter)
        card_layout.addWidget(self._recovery_wallet_button, alignment=Qt.AlignHCenter)
        card_layout.addSpacing(4)
        card_layout.addWidget(self._setup_scan_button, alignment=Qt.AlignHCenter)
        outer.addWidget(card, alignment=Qt.AlignHCenter)

        outer.addStretch()
        return page

    def _build_dashboard_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Dashboard",
            "Übersicht über deine BC2 Wallet und den aktuellen Gerätestatus.",
        )

        cards = QHBoxLayout()
        cards.setSpacing(16)

        device_card = self._card()
        dl = QVBoxLayout(device_card)
        dl.setContentsMargins(20, 18, 20, 18)
        dl.setSpacing(9)
        t = QLabel("Gerät")
        t.setObjectName("CardTitle")
        self._dash_device_state = QLabel("Suche nach Hardware Wallet …")
        self._dash_device_state.setObjectName("StrongMuted")
        self._dash_device_name = QLabel("—")
        self._dash_device_name.setObjectName("SmallMuted")
        manage = QPushButton("Gerät verwalten  →")
        manage.setObjectName("SoftGreenButton")
        manage.clicked.connect(lambda: self._navigate("device"))
        dl.addWidget(t)
        dl.addWidget(self._dash_device_state)
        dl.addWidget(self._dash_device_name)
        dl.addStretch()
        dl.addWidget(manage)

        wallet_card = self._card()
        wl = QVBoxLayout(wallet_card)
        wl.setContentsMargins(20, 18, 20, 18)
        wl.setSpacing(10)
        title = QLabel("Wallet Status")
        title.setObjectName("CardTitle")
        wl.addWidget(title)
        wl.addWidget(self._info_pair("Modus", "Hardware Wallet"))
        wl.addWidget(self._info_pair("Wallet", "Seed sicher auf Hardware eingerichtet"))
        sync_row = QHBoxLayout()
        sync_row.addWidget(QLabel("Synchronisierung"))
        sync_row.addStretch()
        self._dash_sync = QLabel("noch nicht gestartet")
        self._dash_sync.setObjectName("ValueLabel")
        sync_row.addWidget(self._dash_sync)
        wl.addLayout(sync_row)
        wl.addStretch()

        network_card = self._card()
        nl = QVBoxLayout(network_card)
        nl.setContentsMargins(20, 18, 20, 18)
        nl.setSpacing(10)
        nt = QLabel("Netzwerk")
        nt.setObjectName("CardTitle")
        self._dash_server = QLabel(self._electrum_server())
        self._dash_server.setObjectName("ValueLabel")
        self._dash_network = QLabel("Nicht verbunden")
        self._dash_network.setObjectName("StatusNeutral")
        nl.addWidget(nt)
        nl.addWidget(QLabel("Electrum Server"))
        nl.addWidget(self._dash_server)
        nl.addWidget(QLabel("Verbindung"))
        nl.addWidget(self._dash_network)
        nl.addStretch()

        cards.addWidget(device_card, 1)
        cards.addWidget(wallet_card, 1)
        cards.addWidget(network_card, 1)
        outer.addLayout(cards)

        balance = self._card()
        bl = QHBoxLayout(balance)
        bl.setContentsMargins(22, 20, 22, 20)
        bl.setSpacing(30)

        left = QHBoxLayout()
        left.setSpacing(42)

        confirmed = QVBoxLayout()
        confirmed.setSpacing(4)
        bt = QLabel("Bestätigtes Guthaben")
        bt.setObjectName("CardTitle")
        self._dash_confirmed_balance = QLabel("0.00000000 BC2")
        self._dash_confirmed_balance.setObjectName("Balance")
        confirmed_note = QLabel("Blockchain bestätigt")
        confirmed_note.setObjectName("SmallMuted")
        confirmed.addWidget(bt)
        confirmed.addWidget(self._dash_confirmed_balance)
        confirmed.addWidget(confirmed_note)
        confirmed.addStretch()

        pending = QVBoxLayout()
        pending.setSpacing(4)
        pt = QLabel("Unbestätigt")
        pt.setObjectName("CardTitle")
        self._dash_unconfirmed_balance = QLabel("0.00000000 BC2")
        self._dash_unconfirmed_balance.setObjectName("PendingBalance")
        self._dash_unconfirmed_note = QLabel("Keine ausstehenden Transaktionen")
        self._dash_unconfirmed_note.setObjectName("SmallMuted")
        pending.addWidget(pt)
        pending.addWidget(self._dash_unconfirmed_balance)
        pending.addWidget(self._dash_unconfirmed_note)
        pending.addStretch()

        left.addLayout(confirmed)
        left.addLayout(pending)
        left.addStretch()

        right = QVBoxLayout()
        right.setSpacing(10)
        rt = QLabel("Schnellzugriff")
        rt.setObjectName("CardTitle")
        receive = QPushButton("↓   Empfangen")
        receive.setObjectName("SoftGreenButton")
        receive.clicked.connect(lambda: self._navigate("receive"))
        send = QPushButton("↗   Senden")
        send.setObjectName("SoftBlueButton")
        send.clicked.connect(lambda: self._navigate("send"))
        device = QPushButton("▣   Gerät prüfen")
        device.setObjectName("SoftButton")
        device.clicked.connect(lambda: self._navigate("device"))
        right.addWidget(rt)
        right.addWidget(receive)
        right.addWidget(send)
        right.addWidget(device)
        right.addStretch()

        bl.addLayout(left, 2)
        bl.addLayout(right, 1)
        outer.addWidget(balance)

        tx = self._card()
        tl = QVBoxLayout(tx)
        tl.setContentsMargins(22, 18, 22, 18)
        tx_title = QLabel("Letzte Transaktionen")
        tx_title.setObjectName("CardTitle")
        empty = QLabel("Keine Transaktionen vorhanden\nDeine Transaktionen werden später hier angezeigt.")
        empty.setObjectName("EmptyState")
        empty.setAlignment(Qt.AlignCenter)
        all_btn = QPushButton("Alle Transaktionen anzeigen")
        all_btn.setObjectName("OutlineButton")
        all_btn.clicked.connect(lambda: self._navigate("transactions"))
        tl.addWidget(tx_title)
        tl.addStretch()
        tl.addWidget(empty)
        tl.addWidget(all_btn, alignment=Qt.AlignHCenter)
        tl.addStretch()
        outer.addWidget(tx, 1)

        outer.addWidget(self._safety_banner())
        return page

    def _build_receive_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Empfangen",
            "Eine neue BC2 Empfangsadresse wird erst nach Bestätigung auf der Hardware angezeigt.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(16)

        title = QLabel("BC2 empfangen")
        title.setObjectName("SectionTitle")
        self._receive_state = QLabel(
            "Verbinde deine Hardware Wallet, um eine Empfangsadresse anzufordern."
        )
        self._receive_state.setObjectName("BodyText")
        self._receive_state.setWordWrap(True)

        self._receive_button = QPushButton("Empfangsadresse anfordern")
        self._receive_button.setObjectName("PrimaryButton")
        self._receive_button.clicked.connect(self._request_receive)

        self._receive_result = QFrame()
        self._receive_result.setObjectName("ResultPanel")
        rr = QVBoxLayout(self._receive_result)
        rr.setContentsMargins(18, 16, 18, 16)
        self._receive_result_title = QLabel("Noch keine Adresse")
        self._receive_result_title.setObjectName("CardTitle")
        self._receive_result_text = QLabel(
            "Aus Sicherheitsgründen zeigt der Desktop keine erfundene Adresse an."
        )
        self._receive_result_text.setObjectName("SmallMuted")
        self._receive_result_text.setWordWrap(True)

        self._receive_qr = QLabel()
        self._receive_qr.setObjectName("ReceiveQr")
        self._receive_qr.setAlignment(Qt.AlignCenter)
        self._receive_qr.setFixedSize(220, 220)
        self._receive_qr.setVisible(False)

        self._receive_copy_button = QPushButton("Adresse kopieren")
        self._receive_copy_button.setObjectName("OutlineButton")
        self._receive_copy_button.setVisible(False)
        self._receive_copy_button.clicked.connect(self._copy_receive_address)

        rr.addWidget(self._receive_result_title)
        rr.addWidget(self._receive_qr, alignment=Qt.AlignLeft)
        rr.addWidget(self._receive_result_text)
        rr.addWidget(self._receive_copy_button, alignment=Qt.AlignLeft)

        layout.addWidget(title)
        layout.addWidget(self._receive_state)
        layout.addWidget(self._receive_button, alignment=Qt.AlignLeft)
        layout.addWidget(self._receive_result)
        layout.addStretch()

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())
        return page

    def _build_send_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Senden",
            "Transaktionen werden auf dem Desktop vorbereitet und müssen auf der Hardware geprüft werden.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(12)

        title = QLabel("BC2 senden")
        title.setObjectName("SectionTitle")
        layout.addWidget(title)

        layout.addWidget(self._form_label("Empfängeradresse"))
        self._send_address = QLineEdit()
        self._send_address.setPlaceholderText("BC2-Adresse eingeben")
        self._send_address.setObjectName("Input")
        layout.addWidget(self._send_address)

        layout.addWidget(self._form_label("Betrag"))
        self._send_amount = QLineEdit()
        self._send_amount.setPlaceholderText("0.00000000")
        validator = QDoubleValidator(0.0, 999999999.0, 8, self)
        validator.setNotation(QDoubleValidator.StandardNotation)
        self._send_amount.setValidator(validator)
        self._send_amount.setObjectName("Input")
        layout.addWidget(self._send_amount)

        self._send_error = QLabel("")
        self._send_error.setObjectName("ErrorText")
        self._send_error.setWordWrap(True)
        layout.addWidget(self._send_error)

        next_btn = QPushButton("Weiter")
        next_btn.setObjectName("PrimaryButton")
        next_btn.clicked.connect(self._prepare_send)
        layout.addWidget(next_btn, alignment=Qt.AlignLeft)
        layout.addStretch()

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())
        return page

    def _build_transactions_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Transaktionen",
            "Hier erscheinen deine BC2 Transaktionen nach der Wallet-Synchronisierung.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(24, 22, 24, 22)

        title = QLabel("Transaktionsverlauf")
        title.setObjectName("SectionTitle")
        empty = QLabel(
            "Keine Transaktionen vorhanden\n\n"
            "Die Blockchain-Synchronisierung wird in einem folgenden Sprint an diese Seite angebunden."
        )
        empty.setObjectName("EmptyState")
        empty.setAlignment(Qt.AlignCenter)
        empty.setWordWrap(True)

        layout.addWidget(title)
        layout.addStretch()
        layout.addWidget(empty)
        layout.addStretch()

        outer.addWidget(card, 1)
        return page

    def _build_device_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Hardware Wallet",
            "Verbindung und Eigenschaften deiner BC2 Hardware Wallet.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(26, 24, 26, 24)
        layout.setSpacing(20)

        header = QHBoxLayout()
        self._device_status_icon = QLabel("…")
        self._device_status_icon.setAlignment(Qt.AlignCenter)
        self._device_status_icon.setFixedSize(40, 40)
        self._device_status_icon.setObjectName("StatusIconScanning")

        hs = QVBoxLayout()
        self._device_status = QLabel("Suche nach BC2 Hardware Wallet …")
        self._device_status.setObjectName("SectionTitle")
        self._device_detail = QLabel("Serielle Geräte werden geprüft.")
        self._device_detail.setObjectName("SmallMuted")
        hs.addWidget(self._device_status)
        hs.addWidget(self._device_detail)

        self._device_badge = QLabel("Suche …")
        self._device_badge.setObjectName("ConnectionBadgeScanning")
        self._device_badge.setAlignment(Qt.AlignCenter)
        self._device_badge.setFixedSize(150, 36)

        header.addWidget(self._device_status_icon)
        header.addLayout(hs, 1)
        header.addWidget(self._device_badge)
        layout.addLayout(header)

        body = QHBoxLayout()
        body.setSpacing(28)

        visual = QFrame()
        visual.setObjectName("DeviceVisual")
        visual.setFixedSize(220, 285)
        vl = QVBoxLayout(visual)
        vl.setContentsMargins(18, 18, 18, 18)
        self._device_photo = QLabel()
        self._device_photo.setAlignment(Qt.AlignCenter)
        pix = QPixmap(str(self._asset("bc2-device.png")))
        if not pix.isNull():
            self._device_photo.setPixmap(
                pix.scaled(185, 245, Qt.KeepAspectRatio, Qt.SmoothTransformation)
            )
        vl.addWidget(self._device_photo, 1)

        details = QFrame()
        details.setObjectName("DetailsPanel")
        dl = QVBoxLayout(details)
        dl.setContentsMargins(0, 0, 0, 0)
        dl.setSpacing(0)

        self._device_values: dict[str, QLabel] = {}
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
        self._scan_button.clicked.connect(self._device_service.scan)
        layout.addWidget(self._scan_button, alignment=Qt.AlignHCenter)

        self._factory_reset_button = QPushButton("Gerät zurücksetzen (nach Stabilisierung)")
        self._factory_reset_button.setObjectName("DangerButton")
        self._factory_reset_button.setEnabled(False)
        layout.addWidget(self._factory_reset_button, alignment=Qt.AlignHCenter)

        outer.addWidget(card, 1)
        outer.addWidget(self._safety_banner())
        return page

    def _build_settings_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Einstellungen",
            "Nur Einstellungen, die wirklich nötig sind.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(12)

        title = QLabel("Netzwerk")
        title.setObjectName("SectionTitle")
        layout.addWidget(title)

        layout.addWidget(self._form_label("Electrum Server"))
        self._server_input = QLineEdit(self._electrum_server())
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
        layout.addWidget(self._form_label("Vorhandene Empfangsadresse für Sync hinzufügen (optional)"))
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

        save = QPushButton("Einstellungen speichern")
        save.setObjectName("PrimaryButton")
        save.clicked.connect(self._save_settings)
        layout.addWidget(save, alignment=Qt.AlignLeft)

        self._settings_message = QLabel("")
        self._settings_message.setObjectName("SuccessText")
        layout.addWidget(self._settings_message)
        layout.addStretch()

        outer.addWidget(card, 1)
        return page

    def _build_about_page(self) -> QWidget:
        page, outer = self._page_shell(
            "Über",
            "Informationen über die BC2 Cold Wallet.",
        )

        card = self._card()
        layout = QVBoxLayout(card)
        layout.setContentsMargins(28, 26, 28, 26)
        layout.setSpacing(10)

        logo = QLabel()
        logo.setAlignment(Qt.AlignCenter)
        pix = QPixmap(str(self._asset("bc2-logo.png")))
        if not pix.isNull():
            logo.setPixmap(pix.scaled(130, 130, Qt.KeepAspectRatio, Qt.SmoothTransformation))

        name = QLabel("BC2 Cold Wallet")
        name.setObjectName("AboutTitle")
        name.setAlignment(Qt.AlignCenter)
        version = QLabel(f"Desktop-Version {APP_VERSION}")
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
        return page

    def _info_pair(self, key: str, value: str) -> QWidget:
        row = QWidget()
        lay = QHBoxLayout(row)
        lay.setContentsMargins(0, 0, 0, 0)
        k = QLabel(key)
        k.setObjectName("SmallMuted")
        v = QLabel(value)
        v.setObjectName("ValueLabel")
        lay.addWidget(k)
        lay.addStretch()
        lay.addWidget(v)
        return row

    def _form_label(self, text: str) -> QLabel:
        label = QLabel(text)
        label.setObjectName("FormLabel")
        return label

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
        self._device_values[key] = value

        lay.addWidget(label)
        lay.addWidget(value, 1)
        return row

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

    def _navigate(self, key: str) -> None:
        page = self._pages[key]
        self._stack.setCurrentWidget(page)
        if key in self._nav_buttons:
            self._nav_buttons[key].setChecked(True)

    def _electrum_server(self) -> str:
        return str(self._settings.value("electrum/server", DEFAULT_ELECTRUM))

    @Slot()
    def _begin_wallet_creation(self) -> None:
        if self._device is None:
            self._setup_status_title.setText("Hardware Wallet nicht verbunden")
            self._setup_status_text.setText("Verbinde zuerst dein BC2 Gerät per USB.")
            return
        if self._device.wallet_ready and self._device.locked:
            try:
                accepted = begin_unlock(self._device.port)
            except Exception as exc:
                self._setup_status_title.setText("Unlock konnte nicht gestartet werden")
                self._setup_status_text.setText(str(exc))
                return
            if not accepted:
                self._setup_status_title.setText("Gerät ist nicht bereit")
                self._setup_status_text.setText("Unlock konnte auf der Hardware nicht gestartet werden.")
                return
            self._setup_status_title.setText("PIN auf Hardware eingeben")
            self._setup_status_text.setText("Gib jetzt deine 4-stellige PIN ausschließlich auf der BC2 Hardware Wallet ein.")
            self._create_wallet_button.setEnabled(False)
            QTimer.singleShot(1200, self._device_service.scan)
            return
        try:
            accepted = begin_create_wallet(self._device.port)
        except Exception as exc:
            self._setup_status_title.setText("Wallet-Erstellung konnte nicht gestartet werden")
            self._setup_status_text.setText(str(exc))
            return
        if not accepted:
            self._setup_status_title.setText("Gerät ist noch nicht bereit")
            self._setup_status_text.setText("Die Hardware hat die Wallet-Erstellung nicht angenommen.")
            return
        self._setup_in_progress = True
        self._create_wallet_button.setEnabled(False)
        self._recovery_wallet_button.setEnabled(False)
        self._setup_status_title.setText("Einrichtung auf der Hardware")
        self._setup_status_text.setText("Lege zuerst eine 4-stellige PIN auf der Hardware an. Danach erzeugt die Hardware die neue Wallet und zeigt die 12 Recovery-Wörter nur auf dem Gerät.")
        QTimer.singleShot(1200, self._device_service.scan)

    @Slot()
    def _begin_wallet_recovery(self) -> None:
        if self._device is None:
            self._setup_status_title.setText("Hardware Wallet nicht verbunden")
            self._setup_status_text.setText("Verbinde zuerst dein BC2 Gerät per USB.")
            return

        dialog = RecoveryDialog(self)
        if dialog.exec() != QDialog.Accepted or not dialog.mnemonic:
            return
        mnemonic = dialog.mnemonic

        try:
            accepted = begin_recovery(self._device.port)
            if not accepted:
                raise RuntimeError("Die Hardware hat Recovery nicht angenommen.")
            accepted = submit_recovery_mnemonic(self._device.port, mnemonic)
        except Exception as exc:
            self._setup_status_title.setText("Recovery konnte nicht gestartet werden")
            self._setup_status_text.setText(str(exc))
            return
        finally:
            mnemonic = None

        if not accepted:
            self._setup_status_title.setText("Recovery-Daten wurden abgelehnt")
            self._setup_status_text.setText("Die Hardware konnte die Recovery-Phrase nicht übernehmen. Prüfe Firmware-Version und Gerätezustand.")
            return

        self._setup_in_progress = True
        self._create_wallet_button.setEnabled(False)
        self._recovery_wallet_button.setEnabled(False)
        self._setup_status_title.setText("Recovery-Daten übertragen")
        if self._device.wallet_status == 2:
            self._setup_status_text.setText(
                "Gib jetzt auf der Hardware deine bestehende 4-stellige PIN ein. "
                "Nach erfolgreicher PIN-Prüfung wird die Wallet automatisch wiederhergestellt."
            )
        else:
            self._setup_status_text.setText(
                "Lege jetzt auf der Hardware eine neue 4-stellige PIN an und wiederhole sie. "
                "Danach wird die Wallet automatisch wiederhergestellt."
            )
        QTimer.singleShot(1200, self._device_service.scan)

    @Slot()
    def _save_settings(self) -> None:
        value = self._server_input.text().strip()
        if ":" not in value or value.startswith(":") or value.endswith(":"):
            self._settings_message.setObjectName("ErrorText")
            self._settings_message.setText("Bitte einen Server im Format host:port eingeben.")
            self._repolish(self._settings_message)
            return

        host, port = value.rsplit(":", 1)
        if not host.strip() or not port.isdigit() or not (1 <= int(port) <= 65535):
            self._settings_message.setObjectName("ErrorText")
            self._settings_message.setText("Bitte einen gültigen Host und Port eingeben.")
            self._repolish(self._settings_message)
            return

        address = self._sync_address_input.text().strip()
        if address:
            try:
                address_to_scriptpubkey(address)
            except Exception as exc:
                self._settings_message.setObjectName("ErrorText")
                self._settings_message.setText(f"Adresse ungültig: {exc}")
                self._repolish(self._settings_message)
                return
            self._remember_receive_address(address)
            self._sync_address_input.clear()

        self._settings.setValue("electrum/server", value)
        self._dash_server.setText(value)
        self._settings_message.setObjectName("SuccessText")
        self._settings_message.setText("✓ Gespeichert")
        self._repolish(self._settings_message)
        QTimer.singleShot(150, self._sync_balance)


    @Slot()
    def _logout_wallet(self) -> None:
        if self._device is None:
            return
        try:
            if not lock_wallet(self._device.port):
                QMessageBox.warning(
                    self,
                    "Wallet sperren",
                    "Die Hardware Wallet konnte nicht gesperrt werden.",
                )
                return
        except Exception as exc:
            QMessageBox.warning(
                self,
                "Wallet sperren",
                f"Die Hardware Wallet konnte nicht gesperrt werden:\n{exc}",
            )
            return

        self._device = None
        self._sidebar.setVisible(False)
        self._navigate("setup")
        self._setup_status_title.setText("Wallet gesperrt")
        self._setup_status_text.setText(
            "Die Hardware Wallet wurde gesperrt. Zum Fortfahren ist erneut die 4-stellige PIN erforderlich."
        )
        self._create_wallet_button.setText("Unlock Wallet")
        self._create_wallet_button.setEnabled(False)
        self._recovery_wallet_button.setEnabled(False)
        QTimer.singleShot(250, self._device_service.scan)

    @Slot()
    def _request_receive(self) -> None:
        if self._device is None:
            self._receive_result_title.setText("Hardware Wallet erforderlich")
            self._receive_result_text.setText(
                "Schließe zuerst deine BC2 Hardware Wallet an."
            )
            self._receive_copy_button.setVisible(False)
            return

        if not self._device.wallet_ready:
            self._receive_result_title.setText("Wallet noch nicht eingerichtet")
            self._receive_result_text.setText("Richte zuerst deine Wallet vollständig auf der Hardware ein.")
            self._receive_copy_button.setVisible(False)
            return
        if not self._device.unlocked:
            self._receive_result_title.setText("Wallet ist gesperrt")
            self._receive_result_text.setText("Entsperre zuerst die Hardware Wallet mit deiner 4-stelligen PIN.")
            self._receive_copy_button.setVisible(False)
            return

        self._receive_service.request(self._device.port)

    @Slot()
    def _on_receive_started(self) -> None:
        self._receive_button.setEnabled(False)
        self._receive_button.setText("Warte auf Hardware …")
        self._set_receive_qr(None)
        self._receive_result_title.setText("Bestätigung auf Hardware erforderlich")
        self._receive_result_text.setText(
            "Die Empfangsadresse wird ausschließlich auf der Hardware erzeugt."
        )
        self._receive_copy_button.setVisible(False)

    @Slot(str)
    def _on_receive_progress(self, message: str) -> None:
        self._receive_result_title.setText("Adresse auf Hardware prüfen")
        self._receive_result_text.setText(message)

    def _set_receive_qr(self, address: str | None) -> None:
        self._receive_qr.clear()
        self._receive_qr.setVisible(False)

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
            self._receive_qr.setPixmap(
                pixmap.scaled(
                    210,
                    210,
                    Qt.KeepAspectRatio,
                    Qt.SmoothTransformation,
                )
            )
            self._receive_qr.setVisible(True)

    @Slot(int, object)
    def _on_receive_finished(self, status: int, address) -> None:
        self._receive_button.setEnabled(True)
        self._receive_button.setText("Neue Empfangsadresse anfordern")

        if status == 1 and address:
            self._receive_result_title.setText("Empfangsadresse bestätigt")
            self._set_receive_qr(str(address))
            self._receive_result_text.setText(f"Adresse: {address}")
            self._receive_result_text.setTextInteractionFlags(
                Qt.TextSelectableByMouse
            )
            self._receive_copy_button.setVisible(True)
            self._receive_state.setText(
                "Diese Adresse wurde auf deiner BC2 Hardware Wallet geprüft und bestätigt."
            )
            self._remember_receive_address(str(address))
            QTimer.singleShot(100, self._sync_balance)
            return

        self._set_receive_qr(None)
        self._receive_result_title.setText("Adresse nicht freigegeben")
        self._receive_result_text.setText(
            "Die Hardware Wallet hat die Adresse abgelehnt oder konnte sie nicht sicher erzeugen. "
            "Bei einem technischen Ableitungsfehler wird jetzt eine genauere Meldung ausgegeben."
        )
        self._receive_copy_button.setVisible(False)

    @Slot(str)
    def _on_receive_failed(self, message: str) -> None:
        self._receive_button.setEnabled(True)
        self._receive_button.setText("Empfangsadresse anfordern")
        self._set_receive_qr(None)
        self._receive_result_title.setText("Empfangen nicht möglich")
        self._receive_result_text.setText(message)
        self._receive_copy_button.setVisible(False)

    @Slot()
    def _copy_receive_address(self) -> None:
        address = self._receive_result_text.text().strip()
        if address.startswith("Adresse:"):
            address = address.split(":", 1)[1].strip()
        if address:
            QGuiApplication.clipboard().setText(address)
            self._receive_copy_button.setText("✓ Kopiert")
            QTimer.singleShot(
                1400,
                lambda: self._receive_copy_button.setText("Adresse kopieren"),
            )

    @Slot()
    def _prepare_send(self) -> None:
        self._send_error.setText("")
        address = self._send_address.text().strip()
        amount_text = self._send_amount.text().strip().replace(",", ".")

        if not address:
            self._send_error.setText("Bitte eine Empfängeradresse eingeben.")
            return
        if len(address) < 20:
            self._send_error.setText("Die Empfängeradresse ist zu kurz.")
            return

        try:
            amount = float(amount_text)
        except ValueError:
            self._send_error.setText("Bitte einen gültigen Betrag eingeben.")
            return

        if amount <= 0:
            self._send_error.setText("Der Betrag muss größer als 0 sein.")
            return

        if self._device is None:
            self._send_error.setText(
                "Hardware Wallet nicht verbunden. Senden ist ohne Hardware-Bestätigung nicht möglich."
            )
            return

        QMessageBox.information(
            self,
            "Noch nicht signieren",
            "Die Eingaben sind gültig und die Hardware Wallet ist verbunden.\n\n"
            "Die eigentliche Transaktion wird erst freigeschaltet, wenn der sichere "
            "Transaktions- und Signierfluss in Firmware und Desktop vollständig implementiert ist.",
        )

    @Slot()
    def _factory_reset_device(self) -> None:
        QMessageBox.information(
            self,
            "Reset vorübergehend deaktiviert",
            "Der Factory Reset wird erst wieder freigeschaltet, wenn USB-Verbindung und 4-stellige PIN stabil funktionieren.",
        )

    @Slot()
    def _on_scan_started(self) -> None:
        if hasattr(self, "_setup_scan_button"):
            self._setup_scan_button.setEnabled(False)
            self._setup_scan_button.setText("Suche …")
        if self._stack.currentWidget() is self._pages.get("setup"):
            self._setup_status_title.setText("Suche nach BC2 Hardware Wallet …")
            self._setup_status_text.setVisible(True)
            self._setup_status_text.setText(
                "Die App prüft die USB-Verbindung automatisch."
            )
        self._scan_button.setEnabled(False)
        self._scan_button.setText("Suche …")
        self._device_status.setText("Suche nach BC2 Hardware Wallet …")
        self._device_detail.setText("Serielle Geräte werden sicher geprüft.")
        self._device_status_icon.setText("…")
        self._device_status_icon.setObjectName("StatusIconScanning")
        self._device_badge.setText("Suche …")
        self._device_badge.setObjectName("ConnectionBadgeScanning")
        self._sidebar_ready.setText("●  Suche Gerät …")
        self._sidebar_ready.setObjectName("SidebarSearching")
        self._refresh_status_styles()

    @Slot(object)
    def _on_scan_finished(self, result: DiscoveryResult) -> None:
        self._scan_button.setEnabled(True)
        if hasattr(self, "_setup_scan_button"):
            self._setup_scan_button.setEnabled(True)
            self._setup_scan_button.setText("↻   Erneut suchen")
        self._scan_button.setText("↻   Erneut suchen")

        if result.device is None:
            self._factory_reset_button.setEnabled(False)
            self._device = None
            self._device_status.setText("Hardware Wallet nicht verbunden")
            self._device_detail.setText("Schließe die BC2 Hardware Wallet per USB an und suche erneut.")
            self._device_status_icon.setText("!")
            self._device_status_icon.setObjectName("StatusIconOffline")
            self._device_badge.setText("Nicht verbunden")
            self._device_badge.setObjectName("ConnectionBadgeOffline")
            self._sidebar_ready.setText("●  Gerät offline")
            self._sidebar_ready.setObjectName("SidebarOffline")
            self._dash_device_state.setText("Nicht verbunden")
            self._dash_device_name.setText("—")
            self._receive_state.setText("Hardware Wallet nicht verbunden.")
            for value in self._device_values.values():
                value.setText("—")
        else:
            self._factory_reset_button.setEnabled(False)
            self._device = result.device
            d = result.device
            self._device_status.setText("BC2 Hardware Wallet verbunden")
            self._device_detail.setText("Das Gerät antwortet korrekt auf das BC2 USB-Protokoll.")
            self._device_status_icon.setText("✓")
            self._device_status_icon.setObjectName("StatusIconConnected")
            self._device_badge.setText("✓  Verbindung aktiv")
            self._device_badge.setObjectName("ConnectionBadgeConnected")
            self._sidebar_ready.setText("●  Bereit")
            self._sidebar_ready.setObjectName("SidebarReady")
            self._dash_device_state.setText("Verbunden")
            self._dash_device_name.setText(f"{d.device_name}\n{d.hardware_name}")
            self._receive_state.setText(
                "Hardware Wallet verbunden. Empfangsadresse muss auf dem Gerät bestätigt werden."
            )
            values = {
                "Gerät": d.device_name,
                "Hardware": d.hardware_name,
                "Display": d.display_name,
                "Revision": d.firmware_revision,
                "Port": d.port,
                "Gerätestatus": "unbekannt" if d.state is None else str(d.state),
                "Fähigkeiten": d.capabilities_text,
                "Board-Revision": str(d.board_revision),
            }
            for key, value in values.items():
                self._device_values[key].setText(value)

            if d.state == 9:
                self._sidebar.setVisible(False)
                self._navigate("setup")
                self._setup_status_title.setText("Sicherheitsfehler auf der Hardware")
                self._setup_status_text.setVisible(True)
                self._setup_status_text.setText("Die Hardware meldet einen ungültigen Sicherheitszustand. Es wird nichts automatisch überschrieben.")
                self._create_wallet_button.setEnabled(False)
                self._recovery_wallet_button.setEnabled(False)
            elif not d.wallet_ready:
                self._sidebar.setVisible(False)
                self._navigate("setup")
                self._create_wallet_button.setText("Create New Wallet")
                if self._setup_in_progress:
                    self._setup_status_title.setText("Wallet-Einrichtung läuft auf der Hardware")
                    self._setup_status_text.setVisible(True)
                    self._setup_status_text.setText("Folge den Anweisungen auf dem Gerät. Bei Recovery werden 12/24 Wörter am Desktop eingegeben und anschließend auf der Hardware physisch bestätigt. Die PIN bleibt ausschließlich auf der Hardware.")
                    self._create_wallet_button.setEnabled(False)
                    self._recovery_wallet_button.setEnabled(False)
                    QTimer.singleShot(1400, self._device_service.scan)
                else:
                    self._setup_status_title.setText("Noch keine Wallet eingerichtet")
                    self._setup_status_text.setVisible(True)
                    self._setup_status_text.setText("Wähle Create New Wallet oder Recovery Wallet. Eine PIN wird erst nach deiner Auswahl auf der Hardware angelegt.")
                    self._create_wallet_button.setEnabled(True)
                    self._recovery_wallet_button.setEnabled(True)
            elif d.unlocked:
                self._setup_in_progress = False
                self._sidebar.setVisible(True)
                if not self._balance_timer.isActive():
                    self._balance_timer.start()
                QTimer.singleShot(250, self._sync_balance)
                self._create_wallet_button.setText("Create New Wallet")
                self._create_wallet_button.setEnabled(False)
                self._recovery_wallet_button.setEnabled(False)
                if self._stack.currentWidget() is self._pages["setup"]:
                    self._navigate("dashboard")
            else:
                self._sidebar.setVisible(False)
                self._navigate("setup")
                self._create_wallet_button.setText("Unlock Wallet")
                self._create_wallet_button.setEnabled(d.state == 2)
                self._recovery_wallet_button.setEnabled(d.state == 2)
                if d.state == 4:
                    self._setup_status_title.setText("PIN vorübergehend gesperrt")
                    self._setup_status_text.setVisible(True)
                    self._setup_status_text.setText("Zu viele falsche PIN-Versuche. Warte auf die Hardware-Freigabe.")
                elif d.state == 3:
                    self._setup_status_title.setText("PIN-Eingabe läuft")
                    self._setup_status_text.setVisible(True)
                    self._setup_status_text.setText("Gib die 4-stellige PIN auf der Hardware Wallet ein. Recovery kann alternativ über die Desktop-Eingabe gestartet werden.")
                    QTimer.singleShot(1200, self._device_service.scan)
                else:
                    self._setup_status_title.setText("Wallet vorhanden – Gerät gesperrt")
                    self._setup_status_text.clear()
                    self._setup_status_text.setVisible(False)

        if result.device is None:
            self._balance_timer.stop()
            self._dash_network.setText("Nicht verbunden")

        if result.device is None and self._stack.currentWidget() is self._pages["setup"]:
            self._sidebar.setVisible(False)
            self._setup_status_title.setText("Hardware Wallet nicht verbunden")
            self._setup_status_text.setVisible(True)
            self._setup_status_text.setText(
                "Verbinde deine BC2 Hardware Wallet per USB. "
                "Die App sucht automatisch weiter."
            )
            self._create_wallet_button.setEnabled(False)
            self._recovery_wallet_button.setEnabled(False)
            # KISS: retry while onboarding is blocked. DeviceService itself
            # prevents overlapping scans.
            QTimer.singleShot(2000, self._retry_setup_scan)

        self._refresh_status_styles()

    def _known_receive_addresses(self) -> list[str]:
        raw = self._settings.value("wallet/receive_addresses", [])
        if isinstance(raw, str):
            raw = [raw] if raw else []
        return list(dict.fromkeys(str(x).strip() for x in raw if str(x).strip()))

    def _remember_receive_address(self, address: str) -> None:
        addresses = self._known_receive_addresses()
        if address not in addresses:
            addresses.append(address)
            self._settings.setValue("wallet/receive_addresses", addresses)

    def _sync_balance(self) -> None:
        if self._device is None or not self._device.unlocked:
            return
        addresses = self._known_receive_addresses()
        if not addresses:
            self._dash_sync.setText("Keine Adresse bekannt")
            return
        self._electrum_service.sync(self._electrum_server(), addresses)

    @Slot()
    def _on_balance_sync_started(self) -> None:
        self._dash_sync.setText("Synchronisiere …")
        self._dash_network.setText("Verbinde …")

    @staticmethod
    def _format_bc2(sats: int) -> str:
        return f"{sats / 100_000_000:.8f} BC2"

    @Slot(object)
    def _on_balance_sync_finished(self, result: BalanceResult) -> None:
        self._dash_confirmed_balance.setText(self._format_bc2(result.confirmed))
        self._dash_unconfirmed_balance.setText(self._format_bc2(result.unconfirmed))
        self._dash_unconfirmed_note.setText(
            "Wartet auf Blockchain-Bestätigung"
            if result.unconfirmed else
            "Keine ausstehenden Transaktionen"
        )
        self._dash_sync.setText(f"Aktuell · {result.addresses} Adresse(n)")
        self._dash_network.setText("Verbunden")

    @Slot(str)
    def _on_balance_sync_failed(self, message: str) -> None:
        self._dash_sync.setText("Sync fehlgeschlagen")
        self._dash_network.setText("Nicht verbunden")
        self._dash_network.setToolTip(message)

    def _retry_setup_scan(self) -> None:
        if (
            self._device is None
            and self._stack.currentWidget() is self._pages.get("setup")
        ):
            self._device_service.scan()

    def _refresh_status_styles(self) -> None:
        for w in (
            self._device_status_icon,
            self._device_badge,
            self._sidebar_ready,
        ):
            self._repolish(w)

    @staticmethod
    def _repolish(widget: QWidget) -> None:
        widget.style().unpolish(widget)
        widget.style().polish(widget)

    def _apply_styles(self) -> None:
        self.setStyleSheet(f"""
            QMainWindow {{
                background: {BACKGROUND};
            }}
            QWidget#Root, QWidget#Content, QWidget#Page, QStackedWidget#PageStack {{
                background: {BACKGROUND};
                color: {TEXT};
                font-family: "Inter", "Noto Sans", "Segoe UI", Arial, sans-serif;
                font-size: 14px;
            }}
            QLabel#SetupTitle {{
                color: {TEXT};
                font-size: 30px;
                font-weight: 900;
            }}
            QLabel#SetupText {{
                color: {MUTED};
                font-size: 15px;
            }}
            QLabel#SetupWarning {{
                color: {ORANGE_DARK};
                font-size: 13px;
                font-weight: 700;
            }}
            QFrame#Sidebar {{
                background: {SIDEBAR};
                border-right: 1px solid {BORDER};
            }}
            QLabel#BrandName {{
                color: {TEXT};
                font-size: 27px;
                font-weight: 800;
            }}
            QLabel#BrandSubtitle {{
                color: {ORANGE_DARK};
                font-size: 12px;
                font-weight: 800;
            }}
            QPushButton#NavButton {{
                border: none;
                border-radius: 12px;
                text-align: left;
                padding: 0 16px;
                background: transparent;
                color: {MUTED};
                font-size: 15px;
            }}
            QPushButton#NavButton:hover {{
                background: #F0F1F3;
                color: {TEXT};
            }}
            QPushButton#NavButton:checked {{
                background: {ORANGE_SOFT};
                color: {ORANGE_DARK};
                font-weight: 700;
            }}
            QFrame#SidebarStatus {{
                background: {SURFACE};
                border: 1px solid {BORDER};
                border-radius: 12px;
            }}
            QLabel#SidebarVersion, QLabel#SmallMuted, QLabel.SmallMuted {{
                color: {MUTED};
                font-size: 12px;
            }}
            QLabel#SidebarReady {{
                color: {GREEN};
                font-weight: 700;
            }}
            QLabel#SidebarSearching {{
                color: {MUTED};
                font-weight: 700;
            }}
            QLabel#SidebarOffline {{
                color: {RED};
                font-weight: 700;
            }}
            QLabel#PageTitle {{
                color: {TEXT};
                font-size: 30px;
                font-weight: 800;
            }}
            QLabel#PageSubtitle {{
                color: {MUTED};
                font-size: 14px;
            }}
            QFrame#SecurityBadge {{
                background: {SURFACE};
                border: 1px solid {BORDER};
                border-radius: 14px;
            }}
            QLabel#SecurityIcon {{
                background: {GREEN};
                color: white;
                border-radius: 17px;
                font-size: 20px;
                font-weight: 800;
            }}
            QLabel#SecurityPrimary {{
                color: {TEXT};
                font-weight: 700;
            }}
            QLabel#SecuritySecondary {{
                color: {MUTED};
                font-size: 11px;
            }}
            QFrame#Card {{
                background: {SURFACE};
                border: 1px solid {BORDER};
                border-radius: 17px;
            }}
            QLabel#CardTitle {{
                color: {TEXT};
                font-size: 15px;
                font-weight: 800;
            }}
            QLabel#SectionTitle {{
                color: {TEXT};
                font-size: 20px;
                font-weight: 800;
            }}
            QLabel#AboutTitle {{
                color: {TEXT};
                font-size: 26px;
                font-weight: 800;
            }}
            QLabel#StrongMuted {{
                color: {GREEN};
                font-size: 14px;
                font-weight: 800;
            }}
            QLabel#SmallMuted {{
                color: {MUTED};
                font-size: 12px;
            }}
            QLabel#ValueLabel {{
                color: {TEXT};
                font-weight: 500;
            }}
            QLabel#StatusNeutral {{
                color: {MUTED};
                font-weight: 500;
            }}
            QLabel#Balance {{
                color: {TEXT};
                font-size: 27px;
                font-weight: 500;
            }}
            QLabel#PendingBalance {{
                color: {BALANCE};
                font-size: 27px;
                font-weight: 500;
            }}
            QLabel#EmptyState {{
                color: {MUTED};
                font-size: 14px;
                padding: 18px;
            }}
            QPushButton#PrimaryButton {{
                color: white;
                background: {ORANGE};
                border: none;
                border-radius: 10px;
                padding: 11px 20px;
                min-height: 20px;
                font-weight: 800;
            }}
            QPushButton#PrimaryButton:hover {{
                background: {ORANGE_DARK};
            }}
            QPushButton#PrimaryButton:disabled {{
                background: #D5D7DA;
            }}
            QPushButton#RecoveryButton {{
                color: {ORANGE_DARK};
                background: white;
                border: 2px solid {ORANGE};
                border-radius: 10px;
                padding: 11px 20px;
                min-height: 20px;
                font-weight: 800;
            }}
            QPushButton#RecoveryButton:hover {{
                background: {ORANGE_SOFT};
            }}
            QPushButton#RecoveryButton:disabled {{
                color: #A5A5A5;
                background: #F2F3F4;
                border-color: #D5D7DA;
            }}
            QPushButton#SoftGreenButton {{
                color: #248E3B;
                background: {GREEN_SOFT};
                border: 1px solid #C7EBCD;
                border-radius: 9px;
                padding: 10px 14px;
                text-align: left;
                font-weight: 700;
            }}
            QPushButton#SoftBlueButton {{
                color: #2574C8;
                background: {BLUE_SOFT};
                border: 1px solid #D7E9FB;
                border-radius: 9px;
                padding: 10px 14px;
                text-align: left;
                font-weight: 700;
            }}
            QPushButton#SoftButton {{
                color: {TEXT};
                background: #F5F6F8;
                border: 1px solid {BORDER};
                border-radius: 9px;
                padding: 10px 14px;
                text-align: left;
                font-weight: 700;
            }}
            QPushButton#DangerButton {{
                color: {RED};
                background: {RED_SOFT};
                border: 1px solid #E9B8B2;
                border-radius: 10px;
                padding: 10px 18px;
                font-weight: 800;
            }}
            QPushButton#DangerButton:hover {{
                background: #FFE5E1;
            }}
            QPushButton#DangerButton:disabled {{
                color: #A5A5A5;
                background: #F0F0F0;
                border-color: #DDDDDD;
            }}
            QPushButton#OutlineButton {{
                color: {ORANGE_DARK};
                background: white;
                border: 1px solid {ORANGE};
                border-radius: 9px;
                padding: 9px 18px;
                font-weight: 700;
            }}
            QLabel#FormLabel {{
                color: {TEXT};
                font-weight: 700;
                margin-top: 6px;
            }}
            QLineEdit#Input {{
                background: white;
                color: {TEXT};
                border: 1px solid #CCD0D5;
                border-radius: 9px;
                padding: 11px 12px;
                selection-background-color: {ORANGE};
            }}
            QLineEdit#Input:focus {{
                border: 1px solid {ORANGE};
            }}
            QLabel#ErrorText {{
                color: {RED};
                font-weight: 600;
            }}
            QLabel#SuccessText {{
                color: {GREEN};
                font-weight: 700;
            }}
            QLabel#BodyText {{
                color: {MUTED};
                font-size: 14px;
            }}
            QFrame#ResultPanel {{
                background: #F7F8FA;
                border: 1px solid {BORDER};
                border-radius: 11px;
            }}
            QFrame#DeviceVisual {{
                background: #F1F2F4;
                border: 1px solid #D7D9DD;
                border-radius: 18px;
            }}
            QFrame#DetailsPanel {{
                background: transparent;
                border: none;
            }}
            QFrame#DetailRow {{
                background: transparent;
                border: none;
                border-bottom: 1px solid #ECEDEF;
            }}
            QLabel#DetailLabel {{
                color: {MUTED};
                font-size: 13px;
            }}
            QLabel#DetailValue {{
                color: {TEXT};
                font-size: 13px;
                font-weight: 600;
            }}
            QLabel#StatusIconConnected {{
                background: {GREEN_SOFT};
                color: {GREEN};
                border: 2px solid #BFE8C8;
                border-radius: 20px;
                font-size: 23px;
                font-weight: 900;
            }}
            QLabel#StatusIconScanning {{
                background: #F0F1F3;
                color: {MUTED};
                border: 1px solid {BORDER};
                border-radius: 20px;
                font-size: 20px;
                font-weight: 800;
            }}
            QLabel#StatusIconOffline {{
                background: {RED_SOFT};
                color: {RED};
                border: 1px solid #F0C8C4;
                border-radius: 20px;
                font-size: 21px;
                font-weight: 900;
            }}
            QLabel#ConnectionBadgeConnected {{
                background: {GREEN_SOFT};
                color: {GREEN};
                border: 1px solid #8BD49A;
                border-radius: 10px;
                font-weight: 700;
            }}
            QLabel#ConnectionBadgeScanning {{
                background: #F2F3F5;
                color: {MUTED};
                border: 1px solid {BORDER};
                border-radius: 10px;
                font-weight: 700;
            }}
            QLabel#ConnectionBadgeOffline {{
                background: {RED_SOFT};
                color: {RED};
                border: 1px solid #F0C8C4;
                border-radius: 10px;
                font-weight: 700;
            }}
            QFrame#SafetyBanner {{
                background: #FFF9F2;
                border: 1px solid #FFD2A2;
                border-radius: 12px;
            }}
            QLabel#SafetyIcon {{
                color: {ORANGE};
                font-size: 20px;
            }}
            QLabel#SafetyTitle {{
                color: {TEXT};
                font-size: 13px;
                font-weight: 800;
            }}
            QLabel#SafetyText {{
                color: {MUTED};
                font-size: 12px;
            }}
        """)
