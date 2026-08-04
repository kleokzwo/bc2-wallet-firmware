#ifndef BC2_SIMULATOR_MAINWINDOW_H
#define BC2_SIMULATOR_MAINWINDOW_H

#include <QMainWindow>
#include <QVector>

#include "pagerouter.h"

class DeviceController;
class ElectrumClient;
class WatchOnlyModel;
class WatchOnlySync;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QCheckBox;
class QProgressBar;
class Bc2QrWidget;
class QTimer;
class QTableWidget;
class QFrame;
class ThemeManager;
class QResizeEvent;
class QCloseEvent;
class TransactionPage;
class HistoryPage;
class Bc2Button;
class AppSettings;
class HardwareWalletClient;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void showDashboard();
    void showReceive();
    void showWatchOnly();
    void showNetwork();
    void showTransaction();
    void showHistory();
    void showSettings();
    void showAbout();
    void showRecovery();
    void showBackup();
    void showFactoryReset();
    void showLockScreen();
    void submitSimulatorPin();
    void lockDevice();
    void refreshDeviceState();
    void generateNextAddress();
    void confirmAddress();
    void copyAddress();
    void openAddressInExplorer();
    void saveAddressLabel();
    void updateGapLimit(int gapLimit);
    void connectElectrum();
    void startWatchOnlySync();
    void handleNetworkState(const QString &status, bool connected);
    void handleServerVersion(const QString &serverName, const QString &protocolVersion);
    void handleBlockHeight(int height);
    void automaticRefresh();
    void handleNetworkError(const QString &message);
    void handleSyncProgress(int completed, int total, const QString &message);
    void handleSyncFinished(bool success, const QString &message);
    void refreshWatchOnlyView();
    void toggleTheme();

private:
    QWidget *buildShell();
    QWidget *buildDashboard();
    QWidget *buildReceivePage();
    QWidget *buildWatchOnlyPage();
    QWidget *buildNetworkPage();
    QWidget *buildLockPage();
    void openTransactionReview();
    void navigateTo(PageRouter::Page page);
    void updateResponsiveLayout();
    void updateNavigationState(PageRouter::Page page);
    void updateAddress();
    void updateDashboardNetwork(const QString &status, bool connected);
    static QString formatBc2(qint64 satoshis);

    QStackedWidget *pages_ = nullptr;
    QFrame *sidebar_ = nullptr;
    PageRouter *router_ = nullptr;
    ThemeManager *themeManager_ = nullptr;
    AppSettings *appSettings_ = nullptr;
    QPushButton *themeButton_ = nullptr;
    QLabel *addressLabel_ = nullptr;
    QLabel *pathLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    Bc2QrWidget *receiveQr_ = nullptr;
    QLineEdit *addressLabelEdit_ = nullptr;
    QSpinBox *gapLimitSpin_ = nullptr;
    QLabel *gapLimitStatusLabel_ = nullptr;
    QLabel *dashboardNetworkLabel_ = nullptr;
    QLabel *dashboardBalanceLabel_ = nullptr;
    QLabel *networkStatusLabel_ = nullptr;
    QLabel *serverVersionLabel_ = nullptr;
    QLabel *syncStatusLabel_ = nullptr;
    QLabel *syncSummaryLabel_ = nullptr;
    QLabel *confirmedBalanceLabel_ = nullptr;
    QLabel *unconfirmedBalanceLabel_ = nullptr;
    QLabel *lastSyncLabel_ = nullptr;
    QLabel *blockHeightLabel_ = nullptr;
    QLineEdit *hostEdit_ = nullptr;
    QSpinBox *portSpin_ = nullptr;
    QCheckBox *sslCheck_ = nullptr;
    QPushButton *confirmButton_ = nullptr;
    QPushButton *connectButton_ = nullptr;
    QPushButton *syncButton_ = nullptr;
    QProgressBar *syncProgress_ = nullptr;
    QTableWidget *addressTable_ = nullptr;
    QLabel *deviceStateLabel_ = nullptr;
    QLabel *lockMessageLabel_ = nullptr;
    QLineEdit *pinEdit_ = nullptr;
    QPushButton *unlockButton_ = nullptr;
    QPushButton *lockButton_ = nullptr;
    DeviceController *device_ = nullptr;
    HardwareWalletClient *hardwareWallet_ = nullptr;
    ElectrumClient *electrum_ = nullptr;
    WatchOnlyModel *watchModel_ = nullptr;
    WatchOnlySync *watchSync_ = nullptr;
    TransactionPage *transactionPage_ = nullptr;
    HistoryPage *historyPage_ = nullptr;
    QTimer *automaticRefreshTimer_ = nullptr;
    int blockHeight_ = 0;
    QVector<Bc2Button *> navigationButtons_;
    QString currentAddress_;
    unsigned int addressIndex_ = 0U;
};

#endif
