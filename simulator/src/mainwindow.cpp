#include "mainwindow.h"
#include "devicecontroller.h"
#include "electrumclient.h"
#include "epaperwidget.h"
#include "psbtinspectordialog.h"
#include "watchonlymodel.h"
#include "watchonlysync.h"

extern "C" {
#include "bc2_wallet.h"
}

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QDesktopServices>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QPair>
#include <QLineEdit>
#include <QProgressBar>
#include <QTimer>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr const char *kPublishedTestVector =
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
constexpr const char *kDefaultElectrumHost = "infra1.bitcoin-ii.org";
constexpr quint16 kDefaultElectrumPort = 50009;
constexpr const char *kExplorerBaseUrl = "https://explorer.bitcoin-ii.org";

QLabel *heading(const QString &text, int size) {
    auto *label = new QLabel(text);
    QFont font = label->font();
    font.setPointSize(size);
    font.setBold(true);
    label->setFont(font);
    label->setWordWrap(true);
    return label;
}
QLabel *muted(const QString &text) {
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("muted"));
    label->setWordWrap(true);
    return label;
}
QFrame *card() {
    auto *frame = new QFrame;
    frame->setObjectName(QStringLiteral("card"));
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}
QPushButton *secondaryButton(const QString &text) {
    auto *button = new QPushButton(text);
    button->setObjectName(QStringLiteral("secondary"));
    return button;
}
void restyle(QLabel *label, const QString &objectName) {
    label->setObjectName(objectName);
    label->style()->unpolish(label);
    label->style()->polish(label);
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      device_(new DeviceController(this)),
      electrum_(new ElectrumClient(this)),
      watchModel_(new WatchOnlyModel(this)),
      watchSync_(new WatchOnlySync(electrum_, watchModel_, this)) {
    setWindowTitle(QStringLiteral("BC2 Cold Wallet — Simulator 0.7.0"));
    resize(1180, 800);
    setMinimumSize(980, 680);
    watchModel_->buildDemoAccount();
    setCentralWidget(buildShell());

    connect(electrum_, &ElectrumClient::stateChanged, this, &MainWindow::handleNetworkState);
    connect(electrum_, &ElectrumClient::serverVersionReceived, this, &MainWindow::handleServerVersion);
    connect(electrum_, &ElectrumClient::errorOccurred, this, &MainWindow::handleNetworkError);
    connect(watchSync_, &WatchOnlySync::progressChanged, this, &MainWindow::handleSyncProgress);
    connect(watchSync_, &WatchOnlySync::finished, this, &MainWindow::handleSyncFinished);
    connect(watchModel_, &WatchOnlyModel::changed, this, &MainWindow::refreshWatchOnlyView);
    connect(device_, &DeviceController::changed, this, &MainWindow::refreshDeviceState);
    connect(device_, &DeviceController::locked, this, &MainWindow::showLockScreen);
    connect(device_, &DeviceController::unlocked, this, &MainWindow::showDashboard);

    setStyleSheet(R"(
        QMainWindow, QWidget#root { background: #0E1114; }
        QWidget { color: #F4F6F8; font-family: Inter, "Segoe UI", Arial, sans-serif; font-size: 14px; }
        QFrame#sidebar { background: #14181C; border-right: 1px solid #242A30; }
        QFrame#card { background: #181D22; border: 1px solid #2A3138; border-radius: 18px; }
        QLabel#muted { color: #96A0AA; }
        QLabel#address { background: #F2F0E8; color: #111315; border-radius: 14px; padding: 20px; font-family: Consolas, monospace; }
        QLabel#success { color: #D7FF4F; }
        QLabel#error { color: #FF8E8E; }
        QPushButton { background: #D7FF4F; color: #101214; border: 0; border-radius: 11px; padding: 12px 17px; font-weight: 700; }
        QPushButton:hover { background: #E5FF86; }
        QPushButton:disabled { background: #3C444B; color: #89929A; }
        QPushButton#secondary, QPushButton#nav { background: #22282E; color: #F4F6F8; border: 1px solid #343D45; }
        QPushButton#nav { text-align: left; padding: 13px 15px; }
        QPushButton#nav:hover, QPushButton#secondary:hover { background: #2B333A; }
        QLineEdit, QSpinBox, QTableWidget { background: #11161A; border: 1px solid #343D45; border-radius: 9px; padding: 8px; }
        QHeaderView::section { background: #22282E; color: #F4F6F8; padding: 8px; border: 0; }
        QTableWidget { gridline-color: #2A3138; selection-background-color: #2B333A; }
        QProgressBar { border: 1px solid #343D45; border-radius: 7px; background: #11161A; text-align: center; }
        QProgressBar::chunk { background: #D7FF4F; border-radius: 6px; }
    )");

    refreshWatchOnlyView();
    device_->boot();
    refreshDeviceState();
    connectElectrum();
}

QWidget *MainWindow::buildShell() {
    auto *root = new QWidget;
    root->setObjectName(QStringLiteral("root"));
    auto *shell = new QHBoxLayout(root);
    shell->setContentsMargins(0, 0, 0, 0);
    shell->setSpacing(0);

    auto *sidebar = new QFrame;
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(245);
    auto *side = new QVBoxLayout(sidebar);
    side->setContentsMargins(20, 24, 20, 24);
    side->setSpacing(12);
    side->addWidget(heading(QStringLiteral("BC2"), 25));
    side->addWidget(muted(QStringLiteral("Cold Wallet Simulator")));
    side->addSpacing(18);

    const QList<QPair<QString, void (MainWindow::*)()>> navigation = {
        {QStringLiteral("Übersicht"), &MainWindow::showDashboard},
        {QStringLiteral("Empfangen"), &MainWindow::showReceive},
        {QStringLiteral("Watch-only"), &MainWindow::showWatchOnly},
        {QStringLiteral("Netzwerk"), &MainWindow::showNetwork}};
    for (const auto &entry : navigation) {
        auto *button = secondaryButton(entry.first);
        button->setObjectName(QStringLiteral("nav"));
        connect(button, &QPushButton::clicked, this, entry.second);
        side->addWidget(button);
    }
    auto *transactions = secondaryButton(QStringLiteral("Transaktion prüfen"));
    transactions->setObjectName(QStringLiteral("nav"));
    connect(transactions, &QPushButton::clicked, this, [this] {
        if (!device_->isUnlocked()) { showLockScreen(); return; }
        device_->openTransactionReview();
        PsbtInspectorDialog dialog(watchModel_, this);
        dialog.exec();
        device_->cancel();
    });
    side->addWidget(transactions);
    lockButton_ = secondaryButton(QStringLiteral("Gerät sperren"));
    lockButton_->setObjectName(QStringLiteral("nav"));
    connect(lockButton_, &QPushButton::clicked, this, &MainWindow::lockDevice);
    side->addWidget(lockButton_);
    side->addStretch();
    side->addWidget(muted(QStringLiteral("Entwicklungssoftware\nKeine echten Seeds verwenden")));

    pages_ = new QStackedWidget;
    pages_->addWidget(buildDashboard());
    pages_->addWidget(buildReceivePage());
    pages_->addWidget(buildWatchOnlyPage());
    pages_->addWidget(buildNetworkPage());
    pages_->addWidget(buildLockPage());
    shell->addWidget(sidebar);
    shell->addWidget(pages_, 1);
    return root;
}

QWidget *MainWindow::buildDashboard() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(42, 36, 42, 36);
    layout->setSpacing(20);
    layout->addWidget(heading(QStringLiteral("Übersicht"), 26));
    layout->addWidget(muted(QStringLiteral("BC2-only Desktop-Simulator · Testumgebung")));

    auto *stateCard = card();
    auto *stateLayout = new QVBoxLayout(stateCard);
    stateLayout->setContentsMargins(24, 20, 24, 20);
    stateLayout->addWidget(heading(QStringLiteral("Gerätestatus"), 16));
    deviceStateLabel_ = muted(QStringLiteral("Boot"));
    stateLayout->addWidget(deviceStateLabel_);
    layout->addWidget(stateCard);

    auto *balanceCard = card();
    auto *balance = new QVBoxLayout(balanceCard);
    balance->setContentsMargins(24, 24, 24, 24);
    balance->addWidget(muted(QStringLiteral("Watch-only Guthaben")));
    dashboardBalanceLabel_ = heading(QStringLiteral("0,00000000 BC2"), 25);
    balance->addWidget(dashboardBalanceLabel_);
    balance->addWidget(muted(QStringLiteral("Nur öffentliche Blockchain-Daten; keine privaten Schlüssel im Netzwerkmodul.")));
    layout->addWidget(balanceCard);

    auto *deviceCard = card();
    auto *device = new QVBoxLayout(deviceCard);
    device->setContentsMargins(24, 20, 24, 20);
    device->addWidget(heading(QStringLiteral("E-Paper-Gerätereferenz"), 16));
    auto *epaper = new EpaperWidget;
    epaper->setTitle(QStringLiteral("BC2 · WATCH ONLY"));
    epaper->setBody(QStringLiteral("Netzwerkdaten getrennt\nSeed bleibt offline\n296 × 128 Pixel"));
    epaper->setFooter(QStringLiteral("Links: Zurück   Rechts: Bestätigen"));
    device->addWidget(epaper, 0, Qt::AlignLeft);
    layout->addWidget(deviceCard);

    auto *networkCard = card();
    auto *network = new QVBoxLayout(networkCard);
    network->setContentsMargins(22, 20, 22, 20);
    network->addWidget(heading(QStringLiteral("Electrum"), 16));
    dashboardNetworkLabel_ = muted(QStringLiteral("Nicht verbunden"));
    network->addWidget(dashboardNetworkLabel_);
    auto *button = secondaryButton(QStringLiteral("Watch-only öffnen"));
    connect(button, &QPushButton::clicked, this, &MainWindow::showWatchOnly);
    network->addWidget(button);
    layout->addWidget(networkCard);
    layout->addStretch();
    return page;
}

QWidget *MainWindow::buildReceivePage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(42, 36, 42, 36);
    layout->setSpacing(16);
    layout->addWidget(heading(QStringLiteral("Empfangsadresse"), 26));
    layout->addWidget(muted(QStringLiteral("Die Adresse muss später auf dem E-Paper-Gerät vollständig angezeigt und bestätigt werden.")));
    auto *addressCard = card();
    auto *address = new QVBoxLayout(addressCard);
    address->setContentsMargins(24, 24, 24, 24);
    addressLabel_ = new QLabel;
    addressLabel_->setObjectName(QStringLiteral("address"));
    addressLabel_->setWordWrap(true);
    addressLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    address->addWidget(addressLabel_);
    pathLabel_ = muted(QString());
    statusLabel_ = muted(QStringLiteral("Noch nicht bestätigt"));
    address->addWidget(pathLabel_);
    address->addWidget(statusLabel_);
    layout->addWidget(addressCard);
    auto *actions = new QHBoxLayout;
    confirmButton_ = new QPushButton(QStringLiteral("Auf Gerät bestätigen"));
    connect(confirmButton_, &QPushButton::clicked, this, &MainWindow::confirmAddress);
    actions->addWidget(confirmButton_);
    auto *copy = secondaryButton(QStringLiteral("Adresse kopieren"));
    connect(copy, &QPushButton::clicked, this, &MainWindow::copyAddress);
    actions->addWidget(copy);
    auto *explorer = secondaryButton(QStringLiteral("Im Explorer öffnen"));
    connect(explorer, &QPushButton::clicked, this, &MainWindow::openAddressInExplorer);
    actions->addWidget(explorer);
    layout->addLayout(actions);
    auto *next = secondaryButton(QStringLiteral("Nächste Testadresse"));
    connect(next, &QPushButton::clicked, this, &MainWindow::generateNextAddress);
    layout->addWidget(next, 0, Qt::AlignLeft);
    layout->addStretch();
    updateAddress();
    return page;
}

QWidget *MainWindow::buildWatchOnlyPage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(42, 36, 42, 36);
    layout->setSpacing(16);
    layout->addWidget(heading(QStringLiteral("Watch-only-Synchronisation"), 26));
    layout->addWidget(muted(QStringLiteral("Echte Electrum-Abfragen für öffentliche BC2-Adressen, Kontostände, UTXOs und Verlauf.")));

    auto *summaryCard = card();
    auto *summary = new QVBoxLayout(summaryCard);
    summary->setContentsMargins(22, 20, 22, 20);
    syncSummaryLabel_ = heading(QStringLiteral("0,00000000 BC2"), 22);
    syncStatusLabel_ = muted(QStringLiteral("Noch nicht synchronisiert"));
    syncProgress_ = new QProgressBar;
    syncProgress_->setRange(0, 1);
    syncProgress_->setValue(0);
    syncButton_ = new QPushButton(QStringLiteral("Jetzt synchronisieren"));
    connect(syncButton_, &QPushButton::clicked, this, &MainWindow::startWatchOnlySync);
    summary->addWidget(syncSummaryLabel_);
    summary->addWidget(syncStatusLabel_);
    summary->addWidget(syncProgress_);
    summary->addWidget(syncButton_, 0, Qt::AlignLeft);
    layout->addWidget(summaryCard);

    addressTable_ = new QTableWidget;
    addressTable_->setColumnCount(6);
    addressTable_->setHorizontalHeaderLabels({QStringLiteral("Typ"), QStringLiteral("Index"),
                                              QStringLiteral("Adresse"), QStringLiteral("Bestätigt"),
                                              QStringLiteral("Unbestätigt"), QStringLiteral("UTXOs")});
    addressTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    addressTable_->verticalHeader()->setVisible(false);
    addressTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    addressTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(addressTable_, 1);
    return page;
}

QWidget *MainWindow::buildNetworkPage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(42, 36, 42, 36);
    layout->setSpacing(18);
    layout->addWidget(heading(QStringLiteral("BC2 Netzwerk"), 26));
    layout->addWidget(muted(QStringLiteral("Seed und private Schlüssel werden nie an Electrum oder Explorer übertragen.")));
    auto *settings = card();
    auto *form = new QGridLayout(settings);
    form->setContentsMargins(24, 24, 24, 24);
    form->addWidget(new QLabel(QStringLiteral("Electrum-Server")), 0, 0);
    hostEdit_ = new QLineEdit(QString::fromLatin1(kDefaultElectrumHost));
    form->addWidget(hostEdit_, 0, 1);
    form->addWidget(new QLabel(QStringLiteral("Port")), 1, 0);
    portSpin_ = new QSpinBox;
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(kDefaultElectrumPort);
    form->addWidget(portSpin_, 1, 1);
    sslCheck_ = new QCheckBox(QStringLiteral("SSL/TLS-Zertifikat zwingend prüfen"));
    sslCheck_->setChecked(true);
    form->addWidget(sslCheck_, 2, 0, 1, 2);
    connectButton_ = new QPushButton(QStringLiteral("Verbinden"));
    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::connectElectrum);
    form->addWidget(connectButton_, 3, 0, 1, 2);
    layout->addWidget(settings);
    auto *statusCard = card();
    auto *status = new QVBoxLayout(statusCard);
    status->setContentsMargins(24, 22, 24, 22);
    networkStatusLabel_ = muted(QStringLiteral("Nicht verbunden"));
    serverVersionLabel_ = muted(QStringLiteral("Serverversion noch nicht abgefragt"));
    status->addWidget(networkStatusLabel_);
    status->addWidget(serverVersionLabel_);
    layout->addWidget(statusCard);
    auto *explorer = secondaryButton(QStringLiteral("BC2 Block Explorer öffnen"));
    connect(explorer, &QPushButton::clicked, this, [] {
        QDesktopServices::openUrl(QUrl(QString::fromLatin1(kExplorerBaseUrl)));
    });
    layout->addWidget(explorer, 0, Qt::AlignLeft);
    layout->addStretch();
    return page;
}

QWidget *MainWindow::buildLockPage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(120, 90, 120, 90);
    layout->setSpacing(18);
    layout->addStretch();
    layout->addWidget(heading(QStringLiteral("BC2-Gerät gesperrt"), 30));
    layout->addWidget(muted(QStringLiteral("Simulator-PIN: ausschließlich Testwert 2468. Keine echte PIN verwenden.")));
    auto *lockCard = card();
    auto *form = new QVBoxLayout(lockCard);
    form->setContentsMargins(28, 28, 28, 28);
    pinEdit_ = new QLineEdit;
    pinEdit_->setEchoMode(QLineEdit::Password);
    pinEdit_->setMaxLength(12);
    pinEdit_->setPlaceholderText(QStringLiteral("Test-PIN eingeben"));
    unlockButton_ = new QPushButton(QStringLiteral("Entsperren"));
    lockMessageLabel_ = muted(QStringLiteral("Gerät wartet auf Entsperrung"));
    connect(unlockButton_, &QPushButton::clicked, this, &MainWindow::submitSimulatorPin);
    connect(pinEdit_, &QLineEdit::returnPressed, this, &MainWindow::submitSimulatorPin);
    form->addWidget(pinEdit_);
    form->addWidget(unlockButton_);
    form->addWidget(lockMessageLabel_);
    layout->addWidget(lockCard);
    layout->addStretch();
    return page;
}

void MainWindow::showDashboard() { if (device_->isUnlocked()) { device_->noteActivity(); pages_->setCurrentIndex(0); } else showLockScreen(); }
void MainWindow::showReceive() { if (device_->isUnlocked()) { device_->noteActivity(); device_->openReceiveReview(); pages_->setCurrentIndex(1); } else showLockScreen(); }
void MainWindow::showWatchOnly() { if (device_->isUnlocked()) { device_->noteActivity(); pages_->setCurrentIndex(2); } else showLockScreen(); }
void MainWindow::showNetwork() { if (device_->isUnlocked()) { device_->noteActivity(); pages_->setCurrentIndex(3); } else showLockScreen(); }
void MainWindow::showLockScreen() { pages_->setCurrentIndex(4); refreshDeviceState(); }

void MainWindow::submitSimulatorPin() {
    if (device_->state() == BC2_DEVICE_COOLDOWN) { refreshDeviceState(); return; }
    device_->beginUnlock();
    const bool accepted = pinEdit_ != nullptr && pinEdit_->text() == QStringLiteral("2468");
    if (pinEdit_ != nullptr) pinEdit_->clear();
    device_->submitPinResult(accepted);
    refreshDeviceState();
}

void MainWindow::lockDevice() { device_->lock(); }

void MainWindow::refreshDeviceState() {
    const QString state = device_->stateText();
    if (deviceStateLabel_ != nullptr) deviceStateLabel_->setText(QStringLiteral("%1 · automatische Sperre nach 5 Minuten").arg(state));
    if (lockButton_ != nullptr) lockButton_->setEnabled(device_->isUnlocked());
    if (unlockButton_ != nullptr) unlockButton_->setEnabled(device_->state() != BC2_DEVICE_COOLDOWN);
    if (pinEdit_ != nullptr) pinEdit_->setEnabled(device_->state() != BC2_DEVICE_COOLDOWN);
    if (lockMessageLabel_ != nullptr) {
        if (device_->state() == BC2_DEVICE_COOLDOWN) {
            lockMessageLabel_->setText(QStringLiteral("Zu viele Fehlversuche · %1").arg(device_->cooldownText()));
            restyle(lockMessageLabel_, QStringLiteral("error"));
        } else if (device_->state() == BC2_DEVICE_LOCKED || device_->state() == BC2_DEVICE_UNLOCKING) {
            lockMessageLabel_->setText(QStringLiteral("Fehlversuche: %1 von %2").arg(device_->failedAttempts()).arg(device_->maximumAttempts()));
            restyle(lockMessageLabel_, QStringLiteral("muted"));
        }
    }
    if (!device_->isUnlocked() && device_->state() != BC2_DEVICE_BOOT) pages_->setCurrentIndex(4);
}

void MainWindow::generateNextAddress() { ++addressIndex_; updateAddress(); }

void MainWindow::confirmAddress() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->confirm();
    restyle(statusLabel_, QStringLiteral("success"));
    statusLabel_->setText(QStringLiteral("Bestätigt · Simulatoraktion, keine Hardware-Freigabe"));
    confirmButton_->setEnabled(false);
}

void MainWindow::copyAddress() {
    QApplication::clipboard()->setText(currentAddress_);
    statusLabel_->setText(QStringLiteral("Adresse kopiert · weiterhin am Gerät prüfen"));
}

void MainWindow::openAddressInExplorer() {
    if (!currentAddress_.isEmpty()) {
        QDesktopServices::openUrl(QUrl(QStringLiteral("%1/address/%2")
                                          .arg(QString::fromLatin1(kExplorerBaseUrl), currentAddress_)));
    }
}

void MainWindow::connectElectrum() {
    if (serverVersionLabel_ != nullptr) {
        serverVersionLabel_->setText(QStringLiteral("Serverversion wird abgefragt …"));
    }
    electrum_->connectToServer(hostEdit_ != nullptr ? hostEdit_->text() : QString::fromLatin1(kDefaultElectrumHost),
                               static_cast<quint16>(portSpin_ != nullptr ? portSpin_->value() : kDefaultElectrumPort),
                               sslCheck_ == nullptr || sslCheck_->isChecked());
}

void MainWindow::startWatchOnlySync() {
    if (!electrum_->isConnected()) {
        syncStatusLabel_->setText(QStringLiteral("Zuerst eine gültige Electrum-Verbindung herstellen."));
        restyle(syncStatusLabel_, QStringLiteral("error"));
        return;
    }
    syncButton_->setEnabled(false);
    watchSync_->start();
}

void MainWindow::handleNetworkState(const QString &status, bool connected) {
    if (networkStatusLabel_ != nullptr) {
        networkStatusLabel_->setText(status);
        restyle(networkStatusLabel_, connected ? QStringLiteral("success") : QStringLiteral("muted"));
    }
    updateDashboardNetwork(status, connected);
}

void MainWindow::handleServerVersion(const QString &serverName, const QString &protocolVersion) {
    if (serverVersionLabel_ != nullptr) {
        serverVersionLabel_->setText(QStringLiteral("%1 · Electrum-Protokoll %2").arg(serverName, protocolVersion));
    }
}

void MainWindow::handleNetworkError(const QString &message) {
    if (networkStatusLabel_ != nullptr) {
        networkStatusLabel_->setText(QStringLiteral("Fehler: %1").arg(message));
        restyle(networkStatusLabel_, QStringLiteral("error"));
    }
    updateDashboardNetwork(QStringLiteral("Verbindungsfehler"), false);
}

void MainWindow::handleSyncProgress(int completed, int total, const QString &message) {
    syncProgress_->setRange(0, total);
    syncProgress_->setValue(completed);
    syncStatusLabel_->setText(message);
    restyle(syncStatusLabel_, QStringLiteral("muted"));
}

void MainWindow::handleSyncFinished(bool success, const QString &message) {
    syncButton_->setEnabled(true);
    syncStatusLabel_->setText(message);
    restyle(syncStatusLabel_, success ? QStringLiteral("success") : QStringLiteral("error"));
    refreshWatchOnlyView();
}

void MainWindow::refreshWatchOnlyView() {
    const QString total = formatBc2(watchModel_->totalConfirmed() + watchModel_->totalUnconfirmed());
    if (dashboardBalanceLabel_ != nullptr) dashboardBalanceLabel_->setText(total);
    if (syncSummaryLabel_ != nullptr) {
        syncSummaryLabel_->setText(QStringLiteral("%1 · %2 Transaktionen · %3 UTXOs")
                                       .arg(total)
                                       .arg(watchModel_->transactionCount())
                                       .arg(watchModel_->utxoCount()));
    }
    if (addressTable_ == nullptr) return;
    const auto &addresses = watchModel_->addresses();
    addressTable_->setRowCount(static_cast<int>(addresses.size()));
    for (int row = 0; row < static_cast<int>(addresses.size()); ++row) {
        const WatchAddress &address = addresses.at(row);
        addressTable_->setItem(row, 0, new QTableWidgetItem(address.change ? QStringLiteral("Wechsel") : QStringLiteral("Empfang")));
        addressTable_->setItem(row, 1, new QTableWidgetItem(QString::number(address.index)));
        addressTable_->setItem(row, 2, new QTableWidgetItem(address.address));
        addressTable_->setItem(row, 3, new QTableWidgetItem(formatBc2(address.confirmed)));
        addressTable_->setItem(row, 4, new QTableWidgetItem(formatBc2(address.unconfirmed)));
        addressTable_->setItem(row, 5, new QTableWidgetItem(QString::number(address.utxos.size())));
    }
}

void MainWindow::updateDashboardNetwork(const QString &status, bool connected) {
    if (dashboardNetworkLabel_ == nullptr) return;
    dashboardNetworkLabel_->setText(QStringLiteral("%1:%2 · %3")
                                        .arg(QString::fromLatin1(kDefaultElectrumHost))
                                        .arg(kDefaultElectrumPort)
                                        .arg(status));
    restyle(dashboardNetworkLabel_, connected ? QStringLiteral("success") : QStringLiteral("muted"));
}

void MainWindow::updateAddress() {
    bc2_receive_address result{};
    const bc2_wallet_status status = bc2_wallet_receive_address_from_mnemonic(
        kPublishedTestVector, "", bc2_network_mainnet(), 0U, 0U, addressIndex_, &result);
    if (status == BC2_WALLET_OK) {
        currentAddress_ = QString::fromLatin1(result.address);
        addressLabel_->setText(currentAddress_);
        pathLabel_->setText(QStringLiteral("Ableitungspfad: %1").arg(QString::fromLatin1(result.path)));
        restyle(statusLabel_, QStringLiteral("muted"));
        statusLabel_->setText(QStringLiteral("Noch nicht bestätigt"));
        confirmButton_->setEnabled(true);
    } else {
        currentAddress_.clear();
        addressLabel_->setText(QStringLiteral("Adressgenerierung fehlgeschlagen"));
        pathLabel_->setText(QString::fromUtf8(bc2_wallet_status_message(status)));
        statusLabel_->clear();
        confirmButton_->setEnabled(false);
    }
}

QString MainWindow::formatBc2(qint64 satoshis) {
    const bool negative = satoshis < 0;
    const quint64 absolute = negative ? static_cast<quint64>(-(satoshis + 1)) + 1U
                                     : static_cast<quint64>(satoshis);
    return QStringLiteral("%1%2,%3 BC2")
        .arg(negative ? QStringLiteral("-") : QString())
        .arg(absolute / 100000000ULL)
        .arg(absolute % 100000000ULL, 8, 10, QLatin1Char('0'));
}
