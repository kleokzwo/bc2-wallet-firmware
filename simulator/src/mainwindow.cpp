#include "mainwindow.h"
#include "appsettings.h"
#include "devicecontroller.h"
#include "electrumclient.h"
#include "epaperwidget.h"
#include "psbtinspectordialog.h"
#include "thememanager.h"
#include "designtokens.h"
#include "watchonlymodel.h"
#include "watchonlysync.h"
#include "components/bc2button.h"

extern "C" {
#include "bc2_wallet.h"
}

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
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
#include <QResizeEvent>
#include <QTimer>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStyle>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QUrl>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr const char *kPublishedTestVector =
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
constexpr const char *kExplorerBaseUrl = "https://explorer.bitcoin-ii.org";

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
    setWindowTitle(QStringLiteral("BC2 Cold Wallet — Simulator 0.17.2"));
    resize(1180, 800);
    setMinimumSize(DesignTokens::WindowMinimumWidth, DesignTokens::WindowMinimumHeight);
    appSettings_ = new AppSettings;
    themeManager_ = new ThemeManager(qApp, this);
    themeManager_->setTheme(appSettings_->lightThemeEnabled() ? ThemeManager::Theme::Light : ThemeManager::Theme::Dark);
    watchModel_->buildDemoAccount();
    setCentralWidget(buildShell());
    const QByteArray savedGeometry = appSettings_->windowGeometry();
    if (!savedGeometry.isEmpty()) {
        restoreGeometry(savedGeometry);
    }

    connect(electrum_, &ElectrumClient::stateChanged, this, &MainWindow::handleNetworkState);
    connect(electrum_, &ElectrumClient::serverVersionReceived, this, &MainWindow::handleServerVersion);
    connect(electrum_, &ElectrumClient::errorOccurred, this, &MainWindow::handleNetworkError);
    connect(watchSync_, &WatchOnlySync::progressChanged, this, &MainWindow::handleSyncProgress);
    connect(watchSync_, &WatchOnlySync::finished, this, &MainWindow::handleSyncFinished);
    connect(watchModel_, &WatchOnlyModel::changed, this, &MainWindow::refreshWatchOnlyView);
    connect(device_, &DeviceController::changed, this, &MainWindow::refreshDeviceState);
    connect(device_, &DeviceController::locked, this, &MainWindow::showLockScreen);
    connect(device_, &DeviceController::unlocked, this, &MainWindow::showDashboard);



    connect(themeManager_, &ThemeManager::themeChanged, this, [this] {
        auto *button = static_cast<Bc2Button *>(themeButton_);
        if (button != nullptr) {
            const QString label = QStringLiteral("◐  %1").arg(themeManager_->themeName());
            button->setResponsiveText(label, QStringLiteral("◐"));
            button->setCompact(width() < DesignTokens::CompactBreakpoint);
        }
    });

    refreshWatchOnlyView();
    device_->boot();
    refreshDeviceState();
    connectElectrum();
}

MainWindow::~MainWindow() {
    delete appSettings_;
}

void MainWindow::navigateTo(PageRouter::Page page) {
    if (router_ != nullptr) {
        router_->show(page);
    }
}

void MainWindow::toggleTheme() {
    if (themeManager_ == nullptr) {
        return;
    }
    themeManager_->toggleTheme();
    if (appSettings_ != nullptr) {
        appSettings_->setLightThemeEnabled(themeManager_->theme() == ThemeManager::Theme::Light);
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (appSettings_ != nullptr) {
        appSettings_->setWindowGeometry(saveGeometry());
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateResponsiveLayout();
}

void MainWindow::updateResponsiveLayout() {
    const bool compact = width() < DesignTokens::CompactBreakpoint;
    if (sidebar_ != nullptr) {
        sidebar_->setFixedWidth(compact ? DesignTokens::CompactSidebarWidth : DesignTokens::SidebarWidth);
    }

    for (Bc2Button *button : navigationButtons_) {
        if (button != nullptr) {
            button->setCompact(compact);
        }
    }
}

void MainWindow::updateNavigationState(PageRouter::Page page) {
    for (Bc2Button *button : navigationButtons_) {
        if (button == nullptr) {
            continue;
        }

        const QVariant target = button->property("bc2TargetPage");
        button->setActive(target.isValid() && target.toInt() == static_cast<int>(page));
    }
}

void MainWindow::showDashboard() {
    if (!device_->isUnlocked()) {
        showLockScreen();
        return;
    }

    device_->noteActivity();
    navigateTo(PageRouter::Page::Dashboard);
}
void MainWindow::showReceive() {
    if (!device_->isUnlocked()) {
        showLockScreen();
        return;
    }

    device_->noteActivity();
    device_->openReceiveReview();
    navigateTo(PageRouter::Page::Receive);
}
void MainWindow::showWatchOnly() {
    if (!device_->isUnlocked()) {
        showLockScreen();
        return;
    }

    device_->noteActivity();
    navigateTo(PageRouter::Page::WatchOnly);
}
void MainWindow::showNetwork() {
    if (!device_->isUnlocked()) {
        showLockScreen();
        return;
    }

    device_->noteActivity();
    navigateTo(PageRouter::Page::Network);
}

void MainWindow::showTransaction() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::Transaction);
}

void MainWindow::showHistory() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::History);
}

void MainWindow::showSettings() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::Settings);
}

void MainWindow::showAbout() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::About);
}

void MainWindow::showRecovery() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::Recovery);
}

void MainWindow::showBackup() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::Backup);
}

void MainWindow::showFactoryReset() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    navigateTo(PageRouter::Page::FactoryReset);
}

void MainWindow::openTransactionReview() {
    if (!device_->isUnlocked()) { showLockScreen(); return; }
    device_->noteActivity();
    device_->openTransactionReview();
    PsbtInspectorDialog dialog(watchModel_, this);
    dialog.exec();
    device_->cancel();
}

void MainWindow::showLockScreen() {
    navigateTo(PageRouter::Page::Lock);
    refreshDeviceState();
}

void MainWindow::submitSimulatorPin() {
    if (device_->state() == BC2_DEVICE_COOLDOWN) { refreshDeviceState(); return; }
    device_->beginUnlock();
    const bool accepted = pinEdit_ != nullptr && pinEdit_->text() == QStringLiteral("2468");
    if (pinEdit_ != nullptr) pinEdit_->clear();
    device_->submitPinResult(accepted);
    refreshDeviceState();
}

void MainWindow::lockDevice() {
    device_->lock();
}

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
    if (!device_->isUnlocked() && device_->state() != BC2_DEVICE_BOOT) {
        navigateTo(PageRouter::Page::Lock);
    }
}

void MainWindow::generateNextAddress() {
    ++addressIndex_;
    updateAddress();
}

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
    const QString host = hostEdit_ != nullptr ? hostEdit_->text().trimmed() : appSettings_->electrumHost();
    const quint16 port = static_cast<quint16>(portSpin_ != nullptr ? portSpin_->value() : appSettings_->electrumPort());
    const bool sslEnabled = sslCheck_ == nullptr || sslCheck_->isChecked();
    appSettings_->setElectrumServer(host, port, sslEnabled);
    electrum_->connectToServer(host, port, sslEnabled);
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
                                        .arg(appSettings_->electrumHost())
                                        .arg(appSettings_->electrumPort())
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
