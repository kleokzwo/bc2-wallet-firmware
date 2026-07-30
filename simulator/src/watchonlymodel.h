#ifndef BC2_WATCHONLYMODEL_H
#define BC2_WATCHONLYMODEL_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVector>

struct WatchUtxo {
    QString txHash;
    int outputIndex = 0;
    qint64 value = 0;
    int height = 0;
};

struct WatchHistoryItem {
    QString txHash;
    int height = 0;
};

struct WatchAddress {
    QString address;
    QString path;
    QString scriptHash;
    QByteArray scriptPubKey;
    bool change = false;
    unsigned int index = 0;
    qint64 confirmed = 0;
    qint64 unconfirmed = 0;
    QVector<WatchUtxo> utxos;
    QVector<WatchHistoryItem> history;
};

class WatchOnlyModel final : public QObject {
    Q_OBJECT
public:
    explicit WatchOnlyModel(QObject *parent = nullptr);

    bool buildDemoAccount(unsigned int receiveCount = 10U, unsigned int changeCount = 5U);
    const QVector<WatchAddress> &addresses() const;
    qint64 totalConfirmed() const;
    qint64 totalUnconfirmed() const;
    int transactionCount() const;
    int utxoCount() const;

    bool setBalance(const QString &scriptHash, qint64 confirmed, qint64 unconfirmed);
    bool setHistory(const QString &scriptHash, const QVector<WatchHistoryItem> &history);
    bool setUtxos(const QString &scriptHash, const QVector<WatchUtxo> &utxos);
    void clearNetworkData();

signals:
    void changed();

private:
    WatchAddress *find(const QString &scriptHash);
    QVector<WatchAddress> addresses_;
};

#endif
