#ifndef BC2_WATCHONLYSYNC_H
#define BC2_WATCHONLYSYNC_H

#include <QObject>
#include <QSet>
#include <QString>

class ElectrumClient;
class WatchOnlyModel;
class QJsonValue;

class WatchOnlySync final : public QObject {
    Q_OBJECT
public:
    WatchOnlySync(ElectrumClient *client, WatchOnlyModel *model, QObject *parent = nullptr);

    bool isRunning() const;
    void start();
    void cancel();

signals:
    void progressChanged(int completed, int total, const QString &message);
    void finished(bool success, const QString &message);

private slots:
    void handleResponse(qint64 requestId, const QString &method, const QString &scriptHash,
                        const QJsonValue &result);
    void handleFailure(qint64 requestId, const QString &method, const QString &scriptHash,
                       const QString &message);

private:
    void completeRequest(qint64 requestId);
    void finishIfDone();

    ElectrumClient *client_;
    WatchOnlyModel *model_;
    QSet<qint64> pendingIds_;
    int total_ = 0;
    int completed_ = 0;
    int failures_ = 0;
    bool running_ = false;
};

#endif
