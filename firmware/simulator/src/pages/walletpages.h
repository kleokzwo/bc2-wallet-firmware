#ifndef BC2_SIMULATOR_WALLETPAGES_H
#define BC2_SIMULATOR_WALLETPAGES_H

#include <QString>
#include <QWidget>
#include "../watchonlymodel.h"

class QLabel;
class QTableWidget;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;

class TransactionPage final : public QWidget {
    Q_OBJECT
public:
    explicit TransactionPage(WatchOnlyModel *model, QWidget *parent = nullptr);
signals:
    void reviewRequested();
private slots:
    void createDraft();
    void openExistingPsbt();
private:
    WatchOnlyModel *model_ = nullptr;
    QLineEdit *recipientEdit_ = nullptr;
    QDoubleSpinBox *amountSpin_ = nullptr;
    QSpinBox *feeRateSpin_ = nullptr;
    QLabel *previewLabel_ = nullptr;
};

class HistoryPage final : public QWidget {
public:
    explicit HistoryPage(QWidget *parent = nullptr);
    void setTransactions(const QVector<WatchTransaction> &transactions, int blockHeight);
private:
    QTableWidget *table_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};

class SettingsPage final : public QWidget {
public:
    explicit SettingsPage(QWidget *parent = nullptr);
};

class AboutPage final : public QWidget {
public:
    explicit AboutPage(QWidget *parent = nullptr);
};

class RecoveryPage final : public QWidget {
public:
    explicit RecoveryPage(QWidget *parent = nullptr);
};

class BackupPage final : public QWidget {
public:
    explicit BackupPage(QWidget *parent = nullptr);
};

class ErrorPage final : public QWidget {
public:
    explicit ErrorPage(QWidget *parent = nullptr);
    void setMessage(const QString &message);
private:
    QLabel *messageLabel_ = nullptr;
};

class FactoryResetPage final : public QWidget {
public:
    explicit FactoryResetPage(QWidget *parent = nullptr);
};

#endif
