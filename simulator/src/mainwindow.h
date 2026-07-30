#ifndef BC2_SIMULATOR_MAINWINDOW_H
#define BC2_SIMULATOR_MAINWINDOW_H

#include <QMainWindow>

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
class QTimer;
class QTableWidget;

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void showDashboard();
    void showReceive();
    void showWatchOnly();
    void showNetwork();
    void showLockScreen();
    void submitSimulatorPin();
    void lockDevice();
    void refreshDeviceState();
    void generateNextAddress();
    void confirmAddress();
    void copyAddress();
    void openAddressInExplorer();
    void connectElectrum();
    void startWatchOnlySync();
    void handleNetworkState(const QString &status, bool connected);
    void handleServerVersion(const QString &serverName, const QString &protocolVersion);
    void handleNetworkError(const QString &message);
    void handleSyncProgress(int completed, int total, const QString &message);
    void handleSyncFinished(bool success, const QString &message);
    void refreshWatchOnlyView();

private:
    QWidget *buildShell();
    QWidget *buildDashboard();
    QWidget *buildReceivePage();
    QWidget *buildWatchOnlyPage();
    QWidget *buildNetworkPage();
    QWidget *buildLockPage();
    void updateAddress();
    void updateDashboardNetwork(const QString &status, bool connected);
    static QString formatBc2(qint64 satoshis);

    QStackedWidget *pages_ = nullptr;
    QLabel *addressLabel_ = nullptr;
    QLabel *pathLabel_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *dashboardNetworkLabel_ = nullptr;
    QLabel *dashboardBalanceLabel_ = nullptr;
    QLabel *networkStatusLabel_ = nullptr;
    QLabel *serverVersionLabel_ = nullptr;
    QLabel *syncStatusLabel_ = nullptr;
    QLabel *syncSummaryLabel_ = nullptr;
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
    ElectrumClient *electrum_ = nullptr;
    WatchOnlyModel *watchModel_ = nullptr;
    WatchOnlySync *watchSync_ = nullptr;
    QString currentAddress_;
    unsigned int addressIndex_ = 0U;
};

#endif
