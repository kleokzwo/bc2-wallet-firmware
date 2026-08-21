from __future__ import annotations

from pathlib import Path
from PySide6.QtCore import QDateTime, QSettings, QTimer, Qt, Signal, Slot, QSize
from PySide6.QtGui import QPixmap, QIcon
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
    submit_recovery_mnemonic, lock_wallet, get_wallet_id,
)
from bc2.device.model import DeviceInfo
from bc2.services.device_service import DeviceService
from bc2.services.receive_service import ReceiveService
from bc2.services.electrum_service import ElectrumService, BalanceResult, address_to_scriptpubkey
from bc2.services.send_service import SendService
from bc2.services.transaction_service import TransactionService, TransactionEntry
from bc2.ui.recovery_dialog import RecoveryDialog
from bc2.ui.pages.dashboard_page import DashboardPage
from bc2.ui.pages.receiver_page import ReceivePage
from bc2.ui.pages.send_page import SendPage
from bc2.ui.pages.transaction_page import TransactionPage
from bc2.ui.pages.device_page import DevicePage
from bc2.ui.pages.settings_page import SettingsPage
from bc2.ui.pages.about_page import AboutPage
from bc2.wallet_context import WalletContext
from bc2.wallet_cache import WalletCache


APP_VERSION = "0.49.5"
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
        self._wallet_context = WalletContext()
        self._wallet_cache = WalletCache(
            Path.home() / ".cache" / "bc2-cold-wallet"
        )
        self._balance_sync_wallet_id: str | None = None
        self._transaction_sync_wallet_id: str | None = None
        self._setup_in_progress = False
        self._current_send_plan = None
        self._current_signed_transaction = None
        self._transaction_entries = []

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

        self._transaction_service = TransactionService(self)
        self._transaction_service.started.connect(
            self._on_transaction_sync_started
        )
        self._transaction_service.finished.connect(
            self._on_transaction_sync_finished
        )
        self._transaction_service.failed.connect(
            self._on_transaction_sync_failed
        )

        self._send_service = SendService(self)
        self._send_service.started.connect(self._on_send_prepare_started)
        self._send_service.finished.connect(self._on_send_plan_ready)
        self._send_service.failed.connect(self._on_send_prepare_failed)
        self._send_service.review_started.connect(self._on_send_review_started)
        self._send_service.review_progress.connect(self._on_send_review_progress)
        self._send_service.review_finished.connect(self._on_send_review_finished)
        self._send_service.review_failed.connect(self._on_send_review_failed)
        self._send_service.sign_started.connect(self._on_send_sign_started)
        self._send_service.sign_progress.connect(self._on_send_sign_progress)
        self._send_service.sign_finished.connect(self._on_send_sign_finished)
        self._send_service.sign_failed.connect(self._on_send_sign_failed)
        self._send_service.broadcast_started.connect(self._on_send_broadcast_started)
        self._send_service.broadcast_finished.connect(self._on_send_broadcast_finished)
        self._send_service.broadcast_failed.connect(self._on_send_broadcast_failed)

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
    
    def _icon_path(self, name: str) -> str:
        return str(self._asset(f"icons/{name}.svg"))

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
            ("dashboard", "layout-dashboard", "DASHBOARD"),
            ("receive", "arrow-down-to-line", "EMPFANGEN"),
            ("send", "send", "SENDEN"),
            ("transactions", "list", "TRANSAKTIONEN"),
            ("device", "usb", "GERÄT"),
            ("settings", "settings", "EINSTELLUNGEN"),
            ("about", "circle-help", "ÜBER"),
        ]

        group = QButtonGroup(self)
        group.setExclusive(True)

        for key, icon_name, label in nav_items:
            btn = QPushButton(label)
            btn.setObjectName("NavButton")
            btn.setIcon(QIcon(self._icon_path(f"nav-{icon_name}")))
            btn.setIconSize(QSize(19, 19))
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

        self._dashboard_page = DashboardPage(
            icon_path=self._icon_path,
            electrum_server=self._electrum_server(),
        )
        self._dashboard_page.show_transactions_requested.connect(
            lambda: self._navigate("transactions")
        )

        self._receive_page = ReceivePage()
        self._receive_page.request_receive_requested.connect(
            self._request_receive
        )

        self._send_page = SendPage()
        self._send_page.send_requested.connect(
            self._prepare_send
        )
        self._send_page.review_requested.connect(
            self._review_send_plan
        )
        self._send_page.sign_requested.connect(self._sign_send_plan)
        self._send_page.broadcast_requested.connect(self._broadcast_signed_transaction)
        self._send_page.reset_requested.connect(self._reset_send_flow)

        self._transaction_page = TransactionPage()

        self._device_page = DevicePage(asset_path=self._asset)
        self._device_page.scan_requested.connect(
            self._device_service.scan
        )

        self._settings_page = SettingsPage(
            electrum_server=self._electrum_server()
        )
        self._settings_page.save_requested.connect(
            self._save_settings
        )

        self._about_page = AboutPage(
            app_version=APP_VERSION,
            asset_path=self._asset,
        )

        builders = {
            "setup": self._build_setup_page,
            "dashboard": lambda: self._dashboard_page,
            "receive": lambda: self._receive_page,
            "send": lambda: self._send_page,
            "transactions": lambda: self._transaction_page,
            "device": lambda: self._device_page,
            "settings": lambda: self._settings_page,
            "about": lambda: self._about_page,
        }

        for key, builder in builders.items():
            page = builder()
            self._pages[key] = page
            self._stack.addWidget(page)

        layout.addWidget(self._stack)
        return host

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

        self._setup_status_title = QLabel("")
        self._setup_status_title.setObjectName("SectionTitle")
        self._setup_status_title.setAlignment(Qt.AlignCenter)
        self._setup_status_title.setVisible(False)

        self._setup_status_text = QLabel("")
        self._setup_status_text.setObjectName("BodyText")
        self._setup_status_text.setAlignment(Qt.AlignCenter)
        self._setup_status_text.setWordWrap(True)
        self._setup_status_text.setVisible(False)

        self._create_wallet_button = QPushButton("Create New Wallet")
        self._create_wallet_button.setObjectName("PrimaryButton")
        self._create_wallet_button.setCursor(Qt.PointingHandCursor)
        self._create_wallet_button.setMinimumWidth(300)
        self._create_wallet_button.setEnabled(False)
        self._create_wallet_button.clicked.connect(self._begin_wallet_creation)

        self._unlock_wallet_button = QPushButton("Unlock Wallet")
        self._unlock_wallet_button.setObjectName("PrimaryButton")
        self._unlock_wallet_button.setCursor(Qt.PointingHandCursor)
        self._unlock_wallet_button.setMinimumWidth(300)
        self._unlock_wallet_button.setEnabled(False)
        self._unlock_wallet_button.clicked.connect(self._begin_wallet_unlock)

        self._recovery_wallet_button = QPushButton("Recovery Wallet")
        self._recovery_wallet_button.setObjectName("LinkButton")
        self._recovery_wallet_button.setCursor(Qt.PointingHandCursor)
        self._recovery_wallet_button.setEnabled(False)
        self._recovery_wallet_button.clicked.connect(self._begin_wallet_recovery)

        self._setup_scan_button = QPushButton("Erneut verbinden")
        self._setup_scan_button.setObjectName("LinkButton")
        self._setup_scan_button.setCursor(Qt.PointingHandCursor)
        self._setup_scan_button.clicked.connect(self._device_service.scan)

        card_layout.addWidget(self._setup_status_title)
        card_layout.addWidget(self._setup_status_text)
        card_layout.addWidget(self._create_wallet_button, alignment=Qt.AlignHCenter)
        card_layout.addWidget(self._unlock_wallet_button, alignment=Qt.AlignHCenter)
        card_layout.addSpacing(2)

        links = QHBoxLayout()
        links.setSpacing(8)
        links.addStretch()
        links.addWidget(self._recovery_wallet_button)
        separator = QLabel("·")
        separator.setObjectName("SmallMuted")
        links.addWidget(separator)
        links.addWidget(self._setup_scan_button)
        links.addStretch()
        card_layout.addLayout(links)

        outer.addWidget(card, alignment=Qt.AlignHCenter)
        outer.addStretch()
        return page

    def _set_setup_quiet(self) -> None:
        self._setup_status_title.clear()
        self._setup_status_title.setVisible(False)
        self._setup_status_text.clear()
        self._setup_status_text.setVisible(False)

    def _set_setup_message(self, title: str, text: str = "") -> None:
        self._setup_status_title.setText(title)
        self._setup_status_title.setVisible(bool(title))
        self._setup_status_text.setText(text)
        self._setup_status_text.setVisible(bool(text))

    def _navigate(self, key: str) -> None:
        page = self._pages[key]
        self._stack.setCurrentWidget(page)

        if key in self._nav_buttons:
            self._nav_buttons[key].setChecked(True)

        if key == "transactions":
            if self._transaction_entries:
                self._transaction_page.show_transactions(
                    self._transaction_entries
                )
            QTimer.singleShot(0, self._sync_transactions)

    def _electrum_server(self) -> str:
        return str(self._settings.value("electrum/server", DEFAULT_ELECTRUM))

    @Slot()
    def _begin_wallet_creation(self) -> None:
        if self._device is None:
            self._set_setup_message(
                "Hardware Wallet nicht verbunden",
                "Verbinde zuerst dein BC2 Gerät per USB.",
            )
            return

        if self._device.wallet_ready:
            answer = QMessageBox.question(
                self,
                "Create New Wallet",
                "Auf diesem Gerät ist bereits eine Wallet vorhanden.\n\n"
                "Wenn du fortfährst, bestätigst du zuerst die aktuelle 4-stellige "
                "PIN auf der Hardware. Danach legst du eine neue 4-stellige PIN fest "
                "und wiederholst sie. Erst danach wird die aktuelle Wallet vollständig "
                "ersetzt und ein neuer Seed ausschließlich auf der Hardware erzeugt.\n\n"
                "Fortfahren?",
                QMessageBox.Yes | QMessageBox.Cancel,
                QMessageBox.Cancel,
            )
            if answer != QMessageBox.Yes:
                return

        try:
            accepted = begin_create_wallet(self._device.port)
        except Exception as exc:
            self._set_setup_message(
                "Wallet-Erstellung konnte nicht gestartet werden",
                str(exc),
            )
            return
        if not accepted:
            self._set_setup_message(
                "Gerät ist noch nicht bereit",
                "Die Hardware hat die Wallet-Erstellung nicht angenommen.",
            )
            return

        replacing_existing = self._device.wallet_ready
        self._setup_in_progress = True
        self._create_wallet_button.setEnabled(False)
        self._unlock_wallet_button.setEnabled(False)
        self._recovery_wallet_button.setEnabled(False)
        if replacing_existing:
            self._set_setup_message(
                "Bestätigung auf der Hardware",
                "Bestätige zuerst die aktuelle PIN auf der Hardware. Danach legst du "
                "die neue 4-stellige PIN fest und wiederholst sie.",
            )
        else:
            self._set_setup_message(
                "Einrichtung auf der Hardware",
                "Lege zuerst eine 4-stellige PIN auf der Hardware an. Danach erzeugt "
                "die Hardware die neue Wallet und zeigt die 12 Recovery-Wörter nur auf dem Gerät.",
            )
        QTimer.singleShot(1200, self._device_service.scan)

    @Slot()
    def _begin_wallet_unlock(self) -> None:
        if self._device is None:
            self._set_setup_message(
                "Hardware Wallet nicht verbunden",
                "Verbinde zuerst dein BC2 Gerät per USB.",
            )
            return
        if not self._device.wallet_ready:
            self._set_setup_message(
                "Noch keine Wallet eingerichtet",
                "Erstelle zuerst eine neue Wallet oder nutze Recovery Wallet.",
            )
            return
        if self._device.unlocked:
            return

        try:
            accepted = begin_unlock(self._device.port)
        except Exception as exc:
            self._set_setup_message("Unlock konnte nicht gestartet werden", str(exc))
            return
        if not accepted:
            self._set_setup_message(
                "Gerät ist nicht bereit",
                "Unlock konnte auf der Hardware nicht gestartet werden.",
            )
            return

        self._set_setup_quiet()
        self._unlock_wallet_button.setEnabled(False)
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


    @Slot(str, str)
    def _save_settings(self, value: str, address: str) -> None:
        if address:
            try:
                address_to_scriptpubkey(address)
            except Exception as exc:
                self._settings_page.show_error(
                    f"Adresse ungültig: {exc}"
                )
                return

            self._remember_receive_address(address)

        self._settings.setValue("electrum/server", value)
        self._dashboard_page.set_server(value)
        self._settings_page.show_saved()
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

        self._clear_wallet_ui()
        self._wallet_context.deactivate()
        self._device = None
        self._sidebar.setVisible(False)
        self._navigate("setup")
        self._set_setup_quiet()
        self._create_wallet_button.setEnabled(False)
        self._unlock_wallet_button.setEnabled(False)
        self._recovery_wallet_button.setEnabled(False)
        QTimer.singleShot(250, self._device_service.scan)


    @Slot()
    def _request_receive(self) -> None:
        if self._device is None:
            self._receive_page.show_hardware_required()
            return

        if not self._device.wallet_ready:
            self._receive_page.show_wallet_not_ready()
            return

        if not self._device.unlocked:
            self._receive_page.show_wallet_locked()
            return

        self._receive_service.request(self._device.port)

    @Slot()
    def _on_receive_started(self) -> None:
        self._receive_page.show_request_started()

    @Slot(str)
    def _on_receive_progress(self, message: str) -> None:
        self._receive_page.show_progress(message)

    @Slot(int, object)
    def _on_receive_finished(self, status: int, address) -> None:
        confirmed_address = self._receive_page.show_finished(status, address)
        if confirmed_address:
            self._remember_receive_address(confirmed_address)
            QTimer.singleShot(100, self._sync_balance)

    @Slot(str)
    def _on_receive_failed(self, message: str) -> None:
        self._receive_page.show_failed(message)


    @Slot(str, str)
    def _prepare_send(self, address: str, amount_text: str) -> None:
        self._current_send_plan = None
        self._current_signed_transaction = None

        if self._device is None:
            self._send_page.show_hardware_required()
            return

        if not self._device.wallet_ready:
            self._send_page.show_prepare_failed(
                "Wallet noch nicht eingerichtet."
            )
            return

        if not self._device.unlocked:
            self._send_page.show_prepare_failed(
                "Wallet ist gesperrt. Bitte zuerst mit der 4-stelligen PIN entsperren."
            )
            return

        try:
            address_to_scriptpubkey(address)
        except Exception as exc:
            self._send_page.show_prepare_failed(
                f"Empfängeradresse ungültig: {exc}"
            )
            return

        addresses = self._known_receive_addresses()
        if not addresses:
            self._send_page.show_prepare_failed(
                "Keine eigenen Wallet-Adressen gespeichert. "
                "Erzeuge zuerst unter Empfangen eine bestätigte Adresse."
            )
            return

        self._send_service.prepare(
            self._electrum_server(),
            addresses,
            address,
            amount_text,
        )

    @Slot()
    def _on_send_prepare_started(self) -> None:
        self._send_page.show_preparing()

    @Slot(object)
    def _on_send_plan_ready(self, plan) -> None:
        self._current_send_plan = plan
        self._send_page.show_plan(plan)

    @Slot(str)
    def _on_send_prepare_failed(self, message: str) -> None:
        self._send_page.show_prepare_failed(message)

    @Slot()
    def _review_send_plan(self) -> None:
        if self._device is None:
            self._send_page.show_review_failed(
                "Hardware Wallet nicht verbunden."
            )
            return

        if self._current_send_plan is None:
            self._send_page.show_review_failed(
                "Bitte zuerst den Transaktionsentwurf berechnen."
            )
            return

        self._send_service.review(
            self._device.port,
            self._current_send_plan,
        )

    @Slot()
    def _on_send_review_started(self) -> None:
        self._send_page.show_review_started()

    @Slot(str)
    def _on_send_review_progress(self, message: str) -> None:
        self._send_page.show_review_progress(message)

    @Slot(bool)
    def _on_send_review_finished(self, approved: bool) -> None:
        self._send_page.show_review_result(approved)

    @Slot(str)
    def _on_send_review_failed(self, message: str) -> None:
        self._send_page.show_review_failed(message)

    @Slot()
    def _sign_send_plan(self) -> None:
        if self._device is None:
            self._send_page.show_sign_failed(
                "Hardware Wallet nicht verbunden."
            )
            return

        if self._current_send_plan is None:
            self._send_page.show_sign_failed(
                "Bitte zuerst einen Transaktionsentwurf erstellen."
            )
            return

        self._send_service.sign(
            self._device.port,
            self._current_send_plan,
        )

    @Slot()
    def _on_send_sign_started(self) -> None:
        self._send_page.show_sign_started()

    @Slot(str)
    def _on_send_sign_progress(self, message: str) -> None:
        self._send_page.show_sign_progress(message)

    @Slot(object)
    def _on_send_sign_finished(self, signed) -> None:
        self._current_signed_transaction = signed
        self._send_page.show_signed(signed)

    @Slot(str)
    def _on_send_sign_failed(self, message: str) -> None:
        self._send_page.show_sign_failed(message)

    @Slot()
    def _broadcast_signed_transaction(self) -> None:
        if self._current_signed_transaction is None:
            self._send_page.show_broadcast_failed(
                "Keine signierte Transaktion vorhanden."
            )
            return

        self._send_service.broadcast(
            self._electrum_server(),
            self._current_signed_transaction,
        )

    @Slot()
    def _on_send_broadcast_started(self) -> None:
        self._send_page.show_broadcast_started()

    @Slot(str)
    def _on_send_broadcast_finished(self, txid: str) -> None:
        self._send_page.show_broadcast_success(txid)
        QTimer.singleShot(500, self._sync_balance)
        QTimer.singleShot(700, self._sync_transactions)

    @Slot(str)
    def _on_send_broadcast_failed(self, message: str) -> None:
        self._send_page.show_broadcast_failed(message)

    @Slot()
    def _reset_send_flow(self) -> None:
        self._current_send_plan = None
        self._current_signed_transaction = None

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

        # Normal USB discovery is silent on the login page.
        self._device_page.show_scanning()
        self._sidebar_ready.setText("●  Suche Gerät …")
        self._sidebar_ready.setObjectName("SidebarSearching")
        self._refresh_status_styles()

    @Slot(object)
    def _on_scan_finished(self, result: DiscoveryResult) -> None:
        if hasattr(self, "_setup_scan_button"):
            self._setup_scan_button.setEnabled(True)

        if result.device is None:
            self._device = None
            self._device_page.show_offline()
            self._sidebar_ready.setText("●  Gerät offline")
            self._sidebar_ready.setObjectName("SidebarOffline")
            self._dashboard_page.set_device(False)
            self._receive_page.set_device_connected(False)
            self._balance_timer.stop()
            self._dashboard_page.set_network_state("Nicht verbunden")

            if self._stack.currentWidget() is self._pages["setup"]:
                self._sidebar.setVisible(False)
                self._create_wallet_button.setEnabled(False)
                self._unlock_wallet_button.setEnabled(False)
                self._recovery_wallet_button.setEnabled(False)
                self._set_setup_quiet()
                QTimer.singleShot(2000, self._retry_setup_scan)

            self._refresh_status_styles()
            return

        self._device = result.device
        d = result.device
        self._device_page.show_connected(d)
        self._sidebar_ready.setText("●  Bereit")
        self._sidebar_ready.setObjectName("SidebarReady")
        self._dashboard_page.set_device(True)
        self._receive_page.set_device_connected(True)

        if d.state == 9:
            self._sidebar.setVisible(False)
            self._navigate("setup")
            self._create_wallet_button.setEnabled(False)
            self._unlock_wallet_button.setEnabled(False)
            self._recovery_wallet_button.setEnabled(False)
            self._set_setup_message(
                "Sicherheitsfehler auf der Hardware",
                "Die Hardware meldet einen ungültigen Sicherheitszustand. "
                "Es wird nichts automatisch überschrieben.",
            )

        elif not d.wallet_ready:
            self._sidebar.setVisible(False)
            self._navigate("setup")
            self._unlock_wallet_button.setEnabled(False)

            if self._setup_in_progress:
                self._create_wallet_button.setEnabled(False)
                self._recovery_wallet_button.setEnabled(False)
                if not self._setup_status_title.isVisible():
                    self._set_setup_message(
                        "Wallet-Einrichtung läuft auf der Hardware",
                        "Folge den Anweisungen auf dem Gerät.",
                    )
                QTimer.singleShot(1400, self._device_service.scan)
            else:
                self._set_setup_quiet()
                self._create_wallet_button.setEnabled(True)
                self._recovery_wallet_button.setEnabled(True)

        elif d.unlocked:
            if self._wallet_context.active_wallet_id() is None:
                try:
                    wallet_id = get_wallet_id(d.port)
                    if wallet_id is None:
                        raise RuntimeError("Hardware hat keine Wallet-ID geliefert.")
                    self._wallet_context.activate(wallet_id)
                    self._offer_legacy_receive_address_migration(wallet_id)
                    self._load_active_wallet_cache()
                except Exception as exc:
                    self._wallet_context.deactivate()
                    self._sidebar.setVisible(False)
                    self._navigate("setup")
                    self._create_wallet_button.setEnabled(False)
                    self._unlock_wallet_button.setEnabled(False)
                    self._recovery_wallet_button.setEnabled(False)
                    self._set_setup_message(
                        "Wallet-Identität konnte nicht bestätigt werden",
                        str(exc),
                    )
                    QTimer.singleShot(1200, self._device_service.scan)
                    self._refresh_status_styles()
                    return

            self._setup_in_progress = False
            self._set_setup_quiet()
            self._sidebar.setVisible(True)
            if not self._balance_timer.isActive():
                self._balance_timer.start()
            self._create_wallet_button.setEnabled(False)
            self._unlock_wallet_button.setEnabled(False)
            self._recovery_wallet_button.setEnabled(False)
            if self._stack.currentWidget() is self._pages["setup"]:
                self._navigate("dashboard")

            # Let Qt paint the cache-first dashboard before any Electrum work starts.
            QTimer.singleShot(0, self._sync_balance)

        else:
            self._sidebar.setVisible(False)
            self._navigate("setup")
            ready_for_action = d.state == 2
            self._create_wallet_button.setEnabled(ready_for_action)
            self._unlock_wallet_button.setEnabled(ready_for_action)
            self._recovery_wallet_button.setEnabled(ready_for_action)

            if d.state == 4:
                self._set_setup_message(
                    "PIN vorübergehend gesperrt",
                    "Zu viele falsche PIN-Versuche. Warte auf die Hardware-Freigabe.",
                )
            else:
                self._set_setup_quiet()
                if d.state == 3:
                    QTimer.singleShot(1200, self._device_service.scan)

        self._refresh_status_styles()

    def _legacy_receive_addresses(self) -> list[str]:
        """Read pre-v0.48 public addresses without assigning them to a wallet."""
        raw = self._settings.value("wallet/receive_addresses", [])
        if isinstance(raw, str):
            raw = [raw] if raw else []

        result: list[str] = []
        for item in raw:
            value = str(item).strip()
            if not value or value in result:
                continue
            try:
                address_to_scriptpubkey(value)
            except Exception:
                continue
            result.append(value)
        return result

    def _offer_legacy_receive_address_migration(self, wallet_id: str) -> None:
        """Explicitly bind old unscoped addresses to the authenticated wallet.

        Old versions stored receive addresses globally, so automatic assignment
        would risk mixing Wallet A and Wallet B. The user must explicitly confirm
        ownership once. We then remove the legacy global key permanently.
        """
        if self._wallet_cache.load(wallet_id).receive_addresses:
            return

        legacy = self._legacy_receive_addresses()
        if not legacy:
            return

        answer = QMessageBox.question(
            self,
            "Alte Wallet-Daten gefunden",
            "Diese BC2-App hat noch öffentliche Empfangsadressen aus einer älteren "
            "Version gefunden. Damals waren diese Adressen noch keiner Wallet-ID "
            "zugeordnet.\n\n"
            "Gehören diese alten Empfangsadressen zur JETZT entsperrten Wallet?\n\n"
            "Nur mit Ja werden sie dieser Wallet zugeordnet und Balance sowie "
            "Transaktionen anschließend neu über Electrum geladen.",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No,
        )
        if answer != QMessageBox.Yes:
            return

        self._wallet_cache.save_receive_addresses(wallet_id, legacy)
        self._settings.remove("wallet/receive_addresses")
        self._settings.sync()

    def _load_active_wallet_cache(self) -> None:
        """Render only wallet-scoped public cache; never restore Send session state."""
        self._reset_send_session()
        wallet_id = self._wallet_context.active_wallet_id()
        if wallet_id is None:
            self._clear_wallet_ui()
            return

        data = self._wallet_cache.load(wallet_id)

        self._dashboard_page.set_balance(
            self._format_bc2(data.confirmed_balance),
            self._format_bc2(data.unconfirmed_balance),
            bool(data.unconfirmed_balance),
        )

        cached_entries = [
            TransactionEntry(
                txid=row["txid"],
                direction=row["direction"],
                amount=row["amount"],
                height=row["height"],
            )
            for row in data.transactions
        ]
        self._transaction_entries = cached_entries
        self._transaction_page.show_transactions(cached_entries)
        if hasattr(self._dashboard_page, "set_transactions"):
            self._dashboard_page.set_transactions(cached_entries)

        # Receive addresses are retained internally only as public sync metadata
        # for Electrum history/UTXO discovery. They are NEVER restored into the
        # Receive UI. A visible receive address always requires a fresh hardware
        # request and physical confirmation.
        self._receive_page.reset_wallet_view()

        if data.last_sync:
            self._dashboard_page.set_sync_state(
                f"Gespeichert · zuletzt {data.last_sync}"
            )
        elif data.receive_addresses or cached_entries or data.confirmed_balance or data.unconfirmed_balance:
            self._dashboard_page.set_sync_state("Gespeicherte Wallet-Daten")
        else:
            self._dashboard_page.set_sync_state("Noch keine gespeicherten Blockchain-Daten")

        # Cache is useful offline, but it must never pretend that the network is live.
        self._dashboard_page.set_network_state("Nicht verbunden")

    def _reset_send_session(self) -> None:
        """Destroy all ephemeral transaction state between wallet sessions."""
        self._current_send_plan = None
        self._current_signed_transaction = None
        self._send_page.reset_wallet_view()

    def _clear_wallet_ui(self) -> None:
        self._balance_sync_wallet_id = None
        self._transaction_sync_wallet_id = None
        self._transaction_entries = []
        self._reset_send_session()

        self._dashboard_page.set_balance(
            self._format_bc2(0),
            self._format_bc2(0),
            False,
        )
        self._dashboard_page.set_sync_state("Nicht synchronisiert")
        self._transaction_page.show_transactions([])
        if hasattr(self._dashboard_page, "set_transactions"):
            self._dashboard_page.set_transactions([])
        self._receive_page.reset_wallet_view()

    def _known_receive_addresses(self) -> list[str]:
        wallet_id = self._wallet_context.active_wallet_id()
        if wallet_id is None:
            return []
        return list(self._wallet_cache.load(wallet_id).receive_addresses)

    def _remember_receive_address(self, address: str) -> None:
        wallet_id = self._wallet_context.active_wallet_id()
        if wallet_id is None:
            return

        addresses = self._known_receive_addresses()
        value = str(address).strip()
        if value and value not in addresses:
            addresses.append(value)
            self._wallet_cache.save_receive_addresses(wallet_id, addresses)

    def _sync_balance(self) -> None:
        if self._device is None or not self._device.unlocked:
            return

        wallet_id = self._wallet_context.active_wallet_id()
        if wallet_id is None:
            return

        addresses = self._known_receive_addresses()
        if not addresses:
            self._dashboard_page.set_sync_state("Keine Adresse bekannt")
            return

        self._balance_sync_wallet_id = wallet_id
        self._electrum_service.sync(self._electrum_server(), addresses)

        self._transaction_sync_wallet_id = wallet_id
        self._transaction_service.sync(
            self._electrum_server(),
            addresses,
        )

    @Slot()
    def _on_balance_sync_started(self) -> None:
        self._dashboard_page.set_sync_state("Synchronisiere …")
        self._dashboard_page.set_network_state("Verbinde …")

    @staticmethod
    def _format_bc2(sats: int) -> str:
        return f"{sats / 100_000_000:.8f} BC2"

    @Slot(object)
    def _on_balance_sync_finished(self, result: BalanceResult) -> None:
        wallet_id = self._wallet_context.active_wallet_id()
        request_wallet_id = self._balance_sync_wallet_id
        self._balance_sync_wallet_id = None

        if wallet_id is None or request_wallet_id != wallet_id:
            return

        self._wallet_cache.save_balance(
            wallet_id,
            int(result.confirmed),
            int(result.unconfirmed),
        )
        self._wallet_cache.save_last_sync(
            wallet_id,
            QDateTime.currentDateTimeUtc().toString(Qt.ISODate),
        )
        self._dashboard_page.set_balance(
            self._format_bc2(result.confirmed),
            self._format_bc2(result.unconfirmed),
            bool(result.unconfirmed),
        )
        self._dashboard_page.set_sync_state(
            f"Aktuell · {result.addresses} Adresse(n)"
        )
        self._dashboard_page.set_network_state("Verbunden")

    @Slot(str)
    def _on_balance_sync_failed(self, message: str) -> None:
        wallet_id = self._wallet_context.active_wallet_id()
        request_wallet_id = self._balance_sync_wallet_id
        self._balance_sync_wallet_id = None
        if wallet_id is None or request_wallet_id != wallet_id:
            return
        self._dashboard_page.set_sync_state("Sync fehlgeschlagen")
        self._dashboard_page.set_network_state("Nicht verbunden")

    def _sync_transactions(self) -> None:
        if self._device is None or not self._device.unlocked:
            return

        wallet_id = self._wallet_context.active_wallet_id()
        if wallet_id is None:
            return

        addresses = self._known_receive_addresses()
        if not addresses:
            self._transaction_entries = []
            self._transaction_page.show_transactions([])
            if hasattr(self._dashboard_page, "set_transactions"):
                self._dashboard_page.set_transactions([])
            return

        self._transaction_sync_wallet_id = wallet_id
        self._transaction_service.sync(
            self._electrum_server(),
            addresses,
        )

    @Slot()
    def _on_transaction_sync_started(self) -> None:
        # Keep cached rows visible while Electrum refreshes in the background.
        # show_loading() already preserves rows when entries exist, but calling it
        # only for an empty cache makes the cache-first contract explicit.
        if not self._transaction_entries:
            self._transaction_page.show_loading()

    @Slot(object)
    def _on_transaction_sync_finished(self, entries) -> None:
        wallet_id = self._wallet_context.active_wallet_id()
        request_wallet_id = self._transaction_sync_wallet_id
        self._transaction_sync_wallet_id = None

        if wallet_id is None or request_wallet_id != wallet_id:
            return

        self._transaction_entries = list(entries)
        self._wallet_cache.save_transactions(
            wallet_id,
            self._transaction_entries[:100],
        )
        self._transaction_page.show_transactions(self._transaction_entries)

        if hasattr(self._dashboard_page, "set_transactions"):
            self._dashboard_page.set_transactions(self._transaction_entries)

    @Slot(str)
    def _on_transaction_sync_failed(self, message: str) -> None:
        wallet_id = self._wallet_context.active_wallet_id()
        request_wallet_id = self._transaction_sync_wallet_id
        self._transaction_sync_wallet_id = None
        if wallet_id is None or request_wallet_id != wallet_id:
            return

        # Cached transaction history remains useful when Electrum is offline.
        # Only show a full error state when this wallet has no cached history.
        if not self._transaction_entries:
            self._transaction_page.show_error(message)

    def _retry_setup_scan(self) -> None:
        if (
            self._device is None
            and self._stack.currentWidget() is self._pages.get("setup")
        ):
            self._device_service.scan()

    def _refresh_status_styles(self) -> None:
        self._repolish(self._sidebar_ready)

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
                color: {BALANCE};
                font-size: 30px;
                font-weight: 600;
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
                color: {BALANCE};
                font-size: 27px;
                font-weight: 800;
            }}
            QLabel#BrandSubtitle {{
                color: {TEXT};
                font-size: 12px;
                font-weight: 800;
            }}
            QPushButton#NavButton {{
                border: none;
                border-radius: 12px;
                text-align: left;
                padding: 0 16px;
                icon-size: 19px;
                background: transparent;
                color: {MUTED};
                font-size: 15px;
            }}
            QPushButton#NavButton:hover {{
                background: #F6EFF8;
                color: {BALANCE};
            }}
            QPushButton#NavButton:checked {{
                background: #F6EFF8;
                color: {BALANCE};
                font-weight: 600;
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
                color: {BALANCE};
                font-size: 30px;
                font-weight: 600;
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
                color: {BALANCE};
                font-size: 20px;
                font-weight: 600;
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
                background: {BALANCE};
                border: none;
                border-radius: 10px;
                padding: 11px 20px;
                min-height: 20px;
                font-weight: 600;
            }}
            QPushButton#PrimaryButton:hover {{
                background: #6F2B78;
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
                color: {BALANCE};
                background: white;
                border: 1px solid {BALANCE};
                border-radius: 9px;
                padding: 9px 18px;
                font-weight: 600;
            }}
            QPushButton#LinkButton {{
                color: {BALANCE};
                background: transparent;
                border: none;
                padding: 4px 2px;
                font-weight: 500;
                text-decoration: underline;
            }}
            QPushButton#LinkButton:hover {{
                color: #6F2B78;
            }}
            QPushButton#LinkButton:disabled {{
                color: #A5A5A5;
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
            QFrame#DashboardBalanceSection {{
                background: transparent;
                border: none;
            }}
            QLabel#DashboardBalanceLabel {{
                color: {MUTED};
                font-size: 14px;
                font-weight: 500;
            }}
            QLabel#DashboardMainBalance {{
                color: {TEXT};
                font-size: 42px;
                font-weight: 500;
            }}
            QLabel#DashboardBalanceNote {{
                color: {MUTED};
                font-size: 12px;
                font-weight: 400;
            }}
            QLabel#DashboardPendingLabel {{
                color: {MUTED};
                font-size: 13px;
                font-weight: 500;
            }}
            QLabel#DashboardPendingBalance {{
                color: {BALANCE};
                font-size: 20px;
                font-weight: 500;
            }}
            QFrame#DashboardDivider {{
                color: {BORDER};
                background: {BORDER};
                max-height: 1px;
                border: none;
            }}
            QLabel#DashboardSectionTitle {{
                color: {TEXT};
                font-size: 18px;
                font-weight: 600;
            }}
            QLabel#DashboardEmptyText {{
                color: {MUTED};
                font-size: 13px;
                padding: 8px 0;
            }}
            QPushButton#DashboardTextButton {{
                color: {ORANGE_DARK};
                background: transparent;
                border: none;
                padding: 6px 0;
                font-size: 13px;
                font-weight: 600;
            }}
            QPushButton#TopStatusIcon {{
                background: transparent;
                border: none;
                padding: 0;
                margin: 0;
            }}
            QPushButton#TopStatusIcon:hover {{
                background: transparent;
                border: none;
            }}
        """)
