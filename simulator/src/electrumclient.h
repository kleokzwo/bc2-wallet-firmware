#ifndef BC2_SIMULATOR_ELECTRUMCLIENT_H
#define BC2_SIMULATOR_ELECTRUMCLIENT_H

#include <QHash>
#include <QJsonValue>
#include <QObject>
#include <QSslSocket>
#include <QString>

class ElectrumClient final : public QObject {
    Q_OBJECT
public:
    explicit ElectrumClient(QObject *parent = nullptr);

    void connectToServer(const QString &host, quint16 port, bool useSsl);
    void disconnectFromServer();
    bool isConnected() const;
    QString host() const;
    quint16 port() const;

    qint64 requestBalance(const QString &scriptHash);
    qint64 requestHistory(const QString &scriptHash);
    qint64 requestUnspent(const QString &scriptHash);

signals:
    void stateChanged(const QString &status, bool connected);
    void serverVersionReceived(const QString &serverName, const QString &protocolVersion);
    void responseReceived(qint64 requestId, const QString &method, const QString &scriptHash, const QJsonValue &result);
    void requestFailed(qint64 requestId, const QString &method, const QString &scriptHash, const QString &message);
    void errorOccurred(const QString &message);

private slots:
    void onConnected();
    void onEncrypted();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

private:
    struct PendingRequest {
        QString method;
        QString scriptHash;
    };

    void sendVersionRequest();
    qint64 sendScriptHashRequest(const QString &method, const QString &scriptHash);
    qint64 sendRequest(const QString &method, const QJsonArray &params, const QString &scriptHash = {});
    void processLine(const QByteArray &line);
    void failPendingRequests(const QString &message);

    QSslSocket socket_;
    QByteArray receiveBuffer_;
    QString host_;
    quint16 port_ = 0;
    bool useSsl_ = true;
    bool versionRequestSent_ = false;
    qint64 nextRequestId_ = 1;
    QHash<qint64, PendingRequest> pending_;
};

#endif
