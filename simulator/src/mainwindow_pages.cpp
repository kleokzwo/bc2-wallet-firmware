#include "mainwindow.h"
#include "appsettings.h"
#include "devicecontroller.h"
#include "epaperwidget.h"
#include "psbtinspectordialog.h"
#include "watchonlymodel.h"
#include "designtokens.h"
#include "components/bc2button.h"
#include "components/bc2card.h"
#include "components/bc2header.h"
#include "pagerouter.h"
#include "pages/walletpages.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPair>
#include <QProgressBar>
#include <QScrollArea>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr const char *kExplorerBaseUrl = "https://explorer.bitcoin-ii.org";

QLabel *sectionTitle(const QString &text) {
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("sectionTitle"));
    label->setWordWrap(true);
    return label;
}
QLabel *muted(const QString &text) {
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("muted"));
    label->setWordWrap(true);
    return label;
}
}

QWidget *MainWindow::buildShell() {
    auto *root = new QWidget;
    root->setObjectName(QStringLiteral("root"));
    auto *shell = new QHBoxLayout(root);
    shell->setContentsMargins(0, 0, 0, 0);
    shell->setSpacing(0);

    sidebar_ = new QFrame;
    sidebar_->setObjectName(QStringLiteral("sidebar"));
    sidebar_->setFixedWidth(DesignTokens::SidebarWidth);
    auto *side = new QVBoxLayout(sidebar_);
    side->setContentsMargins(18, 22, 18, 18);
    side->setSpacing(10);

    auto *brand = new Bc2Header(QStringLiteral("BC2"), QStringLiteral("Cold Wallet Simulator"));
    side->addWidget(brand);

    auto *environmentBadge = new QLabel(QStringLiteral("● ENTWICKLUNGSUMGEBUNG"));
    environmentBadge->setObjectName(QStringLiteral("environmentBadge"));
    environmentBadge->setToolTip(QStringLiteral("Keine echten Seeds, PINs oder Guthaben verwenden."));
    side->addWidget(environmentBadge);
    side->addSpacing(8);

    auto *navigationHost = new QWidget;
    auto *navigationLayout = new QVBoxLayout(navigationHost);
    navigationLayout->setContentsMargins(0, 0, 0, 0);
    navigationLayout->setSpacing(8);

    struct NavigationEntry {
        PageRouter::Page page;
        QString fullText;
        QString compactText;
        void (MainWindow::*handler)();
    };

    const QList<NavigationEntry> navigation = {
        {PageRouter::Page::Dashboard, QStringLiteral("⌂  Übersicht"), QStringLiteral("⌂"), &MainWindow::showDashboard},
        {PageRouter::Page::Receive, QStringLiteral("↓  Empfangen"), QStringLiteral("↓"), &MainWindow::showReceive},
        {PageRouter::Page::Transaction, QStringLiteral("↗  Transaktion"), QStringLiteral("↗"), &MainWindow::showTransaction},
        {PageRouter::Page::History, QStringLiteral("≡  Verlauf"), QStringLiteral("≡"), &MainWindow::showHistory},
        {PageRouter::Page::WatchOnly, QStringLiteral("◉  Watch-only"), QStringLiteral("◉"), &MainWindow::showWatchOnly},
        {PageRouter::Page::Network, QStringLiteral("⌁  Netzwerk"), QStringLiteral("⌁"), &MainWindow::showNetwork},
        {PageRouter::Page::Settings, QStringLiteral("⚙  Einstellungen"), QStringLiteral("⚙"), &MainWindow::showSettings},
        {PageRouter::Page::About, QStringLiteral("ⓘ  Über"), QStringLiteral("ⓘ"), &MainWindow::showAbout},
        {PageRouter::Page::Recovery, QStringLiteral("↻  Wiederherstellung"), QStringLiteral("↻"), &MainWindow::showRecovery},
        {PageRouter::Page::Backup, QStringLiteral("□  Backup"), QStringLiteral("□"), &MainWindow::showBackup},
        {PageRouter::Page::FactoryReset, QStringLiteral("⚠  Werkseinstellungen"), QStringLiteral("⚠"), &MainWindow::showFactoryReset}};

    for (const NavigationEntry &entry : navigation) {
        auto *button = new Bc2Button(entry.fullText, Bc2Button::Style::Navigation);
        button->setResponsiveText(entry.fullText, entry.compactText);
        button->setProperty("bc2TargetPage", static_cast<int>(entry.page));
        connect(button, &QPushButton::clicked, this, entry.handler);
        navigationLayout->addWidget(button);
        navigationButtons_.append(button);
    }
    navigationLayout->addStretch();

    auto *navigationScroll = new QScrollArea;
    navigationScroll->setObjectName(QStringLiteral("navigationScroll"));
    navigationScroll->setWidgetResizable(true);
    navigationScroll->setFrameShape(QFrame::NoFrame);
    navigationScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    navigationScroll->setWidget(navigationHost);
    side->addWidget(navigationScroll, 1);

    lockButton_ = new Bc2Button(QStringLiteral("⌁  Gerät sperren"), Bc2Button::Style::Navigation);
    auto *responsiveLockButton = static_cast<Bc2Button *>(lockButton_);
    responsiveLockButton->setResponsiveText(QStringLiteral("⌁  Gerät sperren"), QStringLiteral("⌁"));
    responsiveLockButton->setProperty("bc2TargetPage", static_cast<int>(PageRouter::Page::Lock));
    connect(lockButton_, &QPushButton::clicked, this, &MainWindow::lockDevice);
    side->addWidget(lockButton_);
    navigationButtons_.append(responsiveLockButton);

    themeButton_ = new Bc2Button(QStringLiteral("◐  Dark Mode"), Bc2Button::Style::Navigation);
    auto *responsiveThemeButton = static_cast<Bc2Button *>(themeButton_);
    responsiveThemeButton->setResponsiveText(QStringLiteral("◐  Dark Mode"), QStringLiteral("◐"));
    connect(themeButton_, &QPushButton::clicked, this, &MainWindow::toggleTheme);
    side->addWidget(themeButton_);
    navigationButtons_.append(responsiveThemeButton);

    auto *warning = muted(QStringLiteral("Keine echten Seeds verwenden"));
    warning->setAlignment(Qt::AlignCenter);
    side->addWidget(warning);

    pages_ = new QStackedWidget;
    pages_->setObjectName(QStringLiteral("pageStack"));
    router_ = new PageRouter(pages_, this);
    router_->registerPage(PageRouter::Page::Dashboard, buildDashboard());
    router_->registerPage(PageRouter::Page::Receive, buildReceivePage());
    router_->registerPage(PageRouter::Page::WatchOnly, buildWatchOnlyPage());
    router_->registerPage(PageRouter::Page::Network, buildNetworkPage());
    transactionPage_ = new TransactionPage;
    connect(transactionPage_, &TransactionPage::reviewRequested, this, &MainWindow::openTransactionReview);
    router_->registerPage(PageRouter::Page::Transaction, transactionPage_);
    router_->registerPage(PageRouter::Page::History, new HistoryPage);
    router_->registerPage(PageRouter::Page::Settings, new SettingsPage);
    router_->registerPage(PageRouter::Page::About, new AboutPage);
    router_->registerPage(PageRouter::Page::Recovery, new RecoveryPage);
    router_->registerPage(PageRouter::Page::Backup, new BackupPage);
    router_->registerPage(PageRouter::Page::Error, new ErrorPage);
    router_->registerPage(PageRouter::Page::FactoryReset, new FactoryResetPage);
    router_->registerPage(PageRouter::Page::Lock, buildLockPage());
    connect(router_, &PageRouter::pageChanged, this, &MainWindow::updateNavigationState);

    shell->addWidget(sidebar_);
    shell->addWidget(pages_, 1);
    updateResponsiveLayout();
    updateNavigationState(PageRouter::Page::Dashboard);
    return root;
}

QWidget *MainWindow::buildDashboard() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(DesignTokens::PageMarginWide, 36, DesignTokens::PageMarginWide, 36);
    layout->setSpacing(DesignTokens::PageSpacing);
    layout->addWidget(new Bc2Header(QStringLiteral("Übersicht"), QStringLiteral("BC2-only Desktop-Simulator · Testumgebung")));

    auto *stateCard = new Bc2Card;
    auto *stateLayout = new QVBoxLayout(stateCard);
    stateLayout->setContentsMargins(24, 20, 24, 20);
    stateLayout->addWidget(sectionTitle(QStringLiteral("Gerätestatus")));
    deviceStateLabel_ = muted(QStringLiteral("Boot"));
    stateLayout->addWidget(deviceStateLabel_);
    layout->addWidget(stateCard);

    auto *balanceCard = new Bc2Card;
    auto *balance = new QVBoxLayout(balanceCard);
    balance->setContentsMargins(24, 24, 24, 24);
    balance->addWidget(muted(QStringLiteral("Watch-only Guthaben")));
    dashboardBalanceLabel_ = sectionTitle(QStringLiteral("0,00000000 BC2"));
    balance->addWidget(dashboardBalanceLabel_);
    balance->addWidget(muted(QStringLiteral("Nur öffentliche Blockchain-Daten; keine privaten Schlüssel im Netzwerkmodul.")));
    layout->addWidget(balanceCard);

    auto *deviceCard = new Bc2Card;
    auto *device = new QVBoxLayout(deviceCard);
    device->setContentsMargins(24, 20, 24, 20);
    device->addWidget(sectionTitle(QStringLiteral("E-Paper-Gerätereferenz")));
    auto *epaper = new EpaperWidget;
    epaper->setTitle(QStringLiteral("BC2 · WATCH ONLY"));
    epaper->setBody(QStringLiteral("Netzwerkdaten getrennt\nSeed bleibt offline\n296 × 128 Pixel"));
    epaper->setFooter(QStringLiteral("Links: Zurück   Rechts: Bestätigen"));
    device->addWidget(epaper, 0, Qt::AlignLeft);
    layout->addWidget(deviceCard);

    auto *networkCard = new Bc2Card;
    auto *network = new QVBoxLayout(networkCard);
    network->setContentsMargins(22, 20, 22, 20);
    network->addWidget(sectionTitle(QStringLiteral("Electrum")));
    dashboardNetworkLabel_ = muted(QStringLiteral("Nicht verbunden"));
    network->addWidget(dashboardNetworkLabel_);
    auto *button = new Bc2Button(QStringLiteral("Watch-only öffnen"));
    connect(button, &QPushButton::clicked, this, &MainWindow::showWatchOnly);
    network->addWidget(button);
    layout->addWidget(networkCard);
    layout->addStretch();
    return page;
}

QWidget *MainWindow::buildReceivePage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(DesignTokens::PageMarginWide, 36, DesignTokens::PageMarginWide, 36);
    layout->setSpacing(16);
    layout->addWidget(new Bc2Header(QStringLiteral("Empfangsadresse"), QStringLiteral("Die Adresse muss später auf dem E-Paper-Gerät vollständig angezeigt und bestätigt werden.")));
    auto *addressCard = new Bc2Card;
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
    auto *copy = new Bc2Button(QStringLiteral("Adresse kopieren"));
    connect(copy, &QPushButton::clicked, this, &MainWindow::copyAddress);
    actions->addWidget(copy);
    auto *explorer = new Bc2Button(QStringLiteral("Im Explorer öffnen"));
    connect(explorer, &QPushButton::clicked, this, &MainWindow::openAddressInExplorer);
    actions->addWidget(explorer);
    layout->addLayout(actions);
    auto *next = new Bc2Button(QStringLiteral("Nächste Testadresse"));
    connect(next, &QPushButton::clicked, this, &MainWindow::generateNextAddress);
    layout->addWidget(next, 0, Qt::AlignLeft);
    layout->addStretch();
    updateAddress();
    return page;
}

QWidget *MainWindow::buildWatchOnlyPage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(DesignTokens::PageMarginWide, 36, DesignTokens::PageMarginWide, 36);
    layout->setSpacing(16);
    layout->addWidget(new Bc2Header(QStringLiteral("Watch-only-Synchronisation"), QStringLiteral("Echte Electrum-Abfragen für öffentliche BC2-Adressen, Kontostände, UTXOs und Verlauf.")));

    auto *summaryCard = new Bc2Card;
    auto *summary = new QVBoxLayout(summaryCard);
    summary->setContentsMargins(22, 20, 22, 20);
    syncSummaryLabel_ = sectionTitle(QStringLiteral("0,00000000 BC2"));
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
    layout->setContentsMargins(DesignTokens::PageMarginWide, 36, DesignTokens::PageMarginWide, 36);
    layout->setSpacing(18);
    layout->addWidget(new Bc2Header(QStringLiteral("BC2 Netzwerk"), QStringLiteral("Seed und private Schlüssel werden nie an Electrum oder Explorer übertragen.")));
    auto *settings = new Bc2Card;
    auto *form = new QGridLayout(settings);
    form->setContentsMargins(24, 24, 24, 24);
    form->addWidget(new QLabel(QStringLiteral("Electrum-Server")), 0, 0);
    hostEdit_ = new QLineEdit(appSettings_->electrumHost());
    form->addWidget(hostEdit_, 0, 1);
    form->addWidget(new QLabel(QStringLiteral("Port")), 1, 0);
    portSpin_ = new QSpinBox;
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(appSettings_->electrumPort());
    form->addWidget(portSpin_, 1, 1);
    sslCheck_ = new QCheckBox(QStringLiteral("SSL/TLS-Zertifikat zwingend prüfen"));
    sslCheck_->setChecked(appSettings_->electrumSslEnabled());
    form->addWidget(sslCheck_, 2, 0, 1, 2);
    connectButton_ = new QPushButton(QStringLiteral("Verbinden"));
    connect(connectButton_, &QPushButton::clicked, this, &MainWindow::connectElectrum);
    form->addWidget(connectButton_, 3, 0, 1, 2);
    layout->addWidget(settings);
    auto *statusCard = new Bc2Card;
    auto *status = new QVBoxLayout(statusCard);
    status->setContentsMargins(24, 22, 24, 22);
    networkStatusLabel_ = muted(QStringLiteral("Nicht verbunden"));
    serverVersionLabel_ = muted(QStringLiteral("Serverversion noch nicht abgefragt"));
    status->addWidget(networkStatusLabel_);
    status->addWidget(serverVersionLabel_);
    layout->addWidget(statusCard);
    auto *explorer = new Bc2Button(QStringLiteral("BC2 Block Explorer öffnen"));
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
    layout->addWidget(new Bc2Header(QStringLiteral("BC2-Gerät gesperrt"), QStringLiteral("Simulator-PIN: ausschließlich Testwert 2468. Keine echte PIN verwenden.")));
    auto *lockCard = new Bc2Card;
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

