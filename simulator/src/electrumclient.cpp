#include "electrumclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslError>
#include <QStringList>

namespace {
constexpr qsizetype kMaximumResponseBuffer = 4 * 1024 * 1024;
}

ElectrumClient::ElectrumClient(QObject *parent) : QObject(parent) {
    connect(&socket_, &QSslSocket::connected, this, &ElectrumClient::onConnected);
    connect(&socket_, &QSslSocket::encrypted, this, &ElectrumClient::onEncrypted);
    connect(&socket_, &QSslSocket::readyRead, this, &ElectrumClient::onReadyRead);
    connect(&socket_, &QSslSocket::errorOccurred, this, &ElectrumClient::onSocketError);
    connect(&socket_, &QSslSocket::sslErrors, this, &ElectrumClient::onSslErrors);
}

void ElectrumClient::connectToServer(const QString &host, quint16 port, bool useSsl) {
    disconnectFromServer();
    host_ = host.trimmed();
    port_ = port;
    useSsl_ = useSsl;
    versionRequestSent_ = false;
    receiveBuffer_.clear();

    if (host_.isEmpty() || port_ == 0) {
        emit errorOccurred(QStringLiteral("Ungültige Electrum-Serverdaten."));
        return;
    }

    emit stateChanged(QStringLiteral("Verbindung wird aufgebaut …"), false);
    if (useSsl_) {
        socket_.connectToHostEncrypted(host_, port_);
    } else {
        socket_.connectToHost(host_, port_);
    }
}

void ElectrumClient::disconnectFromServer() {
    failPendingRequests(QStringLiteral("Verbindung wurde getrennt."));
    if (socket_.state() != QAbstractSocket::UnconnectedState) {
        socket_.abort();
    }
    versionRequestSent_ = false;
}

bool ElectrumClient::isConnected() const {
    return socket_.state() == QAbstractSocket::ConnectedState && (!useSsl_ || socket_.isEncrypted());
}

QString ElectrumClient::host() const { return host_; }
quint16 ElectrumClient::port() const { return port_; }

qint64 ElectrumClient::requestBalance(const QString &scriptHash) {
    return sendScriptHashRequest(QStringLiteral("blockchain.scripthash.get_balance"), scriptHash);
}

qint64 ElectrumClient::requestHistory(const QString &scriptHash) {
    return sendScriptHashRequest(QStringLiteral("blockchain.scripthash.get_history"), scriptHash);
}

qint64 ElectrumClient::requestUnspent(const QString &scriptHash) {
    return sendScriptHashRequest(QStringLiteral("blockchain.scripthash.listunspent"), scriptHash);
}

void ElectrumClient::onConnected() {
    if (!useSsl_) {
        emit stateChanged(QStringLiteral("Verbunden · unverschlüsselt"), true);
        sendVersionRequest();
    } else {
        emit stateChanged(QStringLiteral("TCP verbunden · SSL wird geprüft …"), false);
    }
}

void ElectrumClient::onEncrypted() {
    emit stateChanged(QStringLiteral("SSL-verschlüsselt verbunden"), true);
    sendVersionRequest();
}

void ElectrumClient::sendVersionRequest() {
    if (versionRequestSent_) {
        return;
    }
    sendRequest(QStringLiteral("server.version"),
                QJsonArray{QStringLiteral("BC2-Cold-Wallet-Simulator/0.5.0"),
                           QJsonArray{QStringLiteral("1.4"), QStringLiteral("1.4.2")}});
    versionRequestSent_ = true;
}

qint64 ElectrumClient::sendScriptHashRequest(const QString &method, const QString &scriptHash) {
    const QString normalized = scriptHash.trimmed().toLower();
    if (normalized.size() != 64) {
        emit errorOccurred(QStringLiteral("Ungültiger Electrum-Scripthash."));
        return -1;
    }
    return sendRequest(method, QJsonArray{normalized}, normalized);
}

qint64 ElectrumClient::sendRequest(const QString &method, const QJsonArray &params, const QString &scriptHash) {
    if (!isConnected()) {
        emit errorOccurred(QStringLiteral("Keine aktive Electrum-Verbindung."));
        return -1;
    }

    const qint64 requestId = nextRequestId_++;
    QJsonObject request;
    request.insert(QStringLiteral("id"), requestId);
    request.insert(QStringLiteral("method"), method);
    request.insert(QStringLiteral("params"), params);

    QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);
    payload.append('\n');
    if (socket_.write(payload) == -1) {
        emit requestFailed(requestId, method, scriptHash, QStringLiteral("Anfrage konnte nicht gesendet werden."));
        return -1;
    }

    pending_.insert(requestId, PendingRequest{method, scriptHash});
    return requestId;
}

void ElectrumClient::onReadyRead() {
    receiveBuffer_.append(socket_.readAll());
    if (receiveBuffer_.size() > kMaximumResponseBuffer) {
        socket_.abort();
        failPendingRequests(QStringLiteral("Electrum-Antwort überschreitet das Sicherheitslimit."));
        emit errorOccurred(QStringLiteral("Electrum-Antwort überschreitet das Sicherheitslimit."));
        return;
    }

    qsizetype newline = -1;
    while ((newline = receiveBuffer_.indexOf('\n')) >= 0) {
        const QByteArray line = receiveBuffer_.left(newline).trimmed();
        receiveBuffer_.remove(0, newline + 1);
        if (!line.isEmpty()) {
            processLine(line);
        }
    }
}

void ElectrumClient::processLine(const QByteArray &line) {
    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        emit errorOccurred(QStringLiteral("Ungültige JSON-Antwort vom Electrum-Server."));
        return;
    }

    const QJsonObject object = document.object();
    const qint64 requestId = object.value(QStringLiteral("id")).toInteger(-1);
    if (requestId < 0 || !pending_.contains(requestId)) {
        return;
    }

    const PendingRequest pending = pending_.take(requestId);
    const QJsonValue error = object.value(QStringLiteral("error"));
    if (!error.isUndefined() && !error.isNull()) {
        QString message = QStringLiteral("Electrum-Protokollfehler");
        if (error.isObject()) {
            message = error.toObject().value(QStringLiteral("message")).toString(message);
        } else if (error.isString()) {
            message = error.toString();
        }
        emit requestFailed(requestId, pending.method, pending.scriptHash, message);
        return;
    }

    const QJsonValue result = object.value(QStringLiteral("result"));
    if (pending.method == QStringLiteral("server.version")) {
        const QJsonArray values = result.toArray();
        if (values.size() >= 2) {
            emit serverVersionReceived(values.at(0).toString(), values.at(1).toString());
        } else {
            emit requestFailed(requestId, pending.method, {}, QStringLiteral("Ungültige server.version-Antwort."));
        }
        return;
    }

    emit responseReceived(requestId, pending.method, pending.scriptHash, result);
}

void ElectrumClient::onSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    const QString message = socket_.errorString();
    failPendingRequests(message);
    emit stateChanged(QStringLiteral("Nicht verbunden"), false);
    emit errorOccurred(message);
}

void ElectrumClient::onSslErrors(const QList<QSslError> &errors) {
    QStringList messages;
    for (const QSslError &error : errors) {
        messages.append(error.errorString());
    }
    socket_.abort();
    const QString message = QStringLiteral("SSL-Zertifikat wurde abgelehnt: %1")
                                .arg(messages.join(QStringLiteral("; ")));
    failPendingRequests(message);
    emit stateChanged(QStringLiteral("SSL-Prüfung fehlgeschlagen"), false);
    emit errorOccurred(message);
}

void ElectrumClient::failPendingRequests(const QString &message) {
    const auto requests = pending_;
    pending_.clear();
    for (auto it = requests.cbegin(); it != requests.cend(); ++it) {
        emit requestFailed(it.key(), it.value().method, it.value().scriptHash, message);
    }
}
