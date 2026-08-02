#include "electrumclient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSslError>
#include <QStringList>

namespace {
constexpr qsizetype kMaximumResponseBuffer = 4 * 1024 * 1024;
constexpr qint64 kRequestTimeoutMs = 15000;
constexpr int kMaximumReconnectDelayMs = 30000;
}

ElectrumClient::ElectrumClient(QObject *parent) : QObject(parent) {
    clock_.start();
    timeoutTimer_.setInterval(1000);
    reconnectTimer_.setSingleShot(true);
    connect(&socket_, &QSslSocket::connected, this, &ElectrumClient::onConnected);
    connect(&socket_, &QSslSocket::encrypted, this, &ElectrumClient::onEncrypted);
    connect(&socket_, &QSslSocket::readyRead, this, &ElectrumClient::onReadyRead);
    connect(&socket_, &QSslSocket::errorOccurred, this, &ElectrumClient::onSocketError);
    connect(&socket_, &QSslSocket::sslErrors, this, &ElectrumClient::onSslErrors);
    connect(&timeoutTimer_, &QTimer::timeout, this, &ElectrumClient::checkRequestTimeouts);
    connect(&reconnectTimer_, &QTimer::timeout, this, &ElectrumClient::reconnect);
    timeoutTimer_.start();
}

void ElectrumClient::connectToServer(const QString &host, quint16 port, bool useSsl) {
    manualDisconnect_ = false;
    reconnectTimer_.stop();
    if (socket_.state() != QAbstractSocket::UnconnectedState) socket_.abort();
    failPendingRequests(QStringLiteral("Verbindung wird neu aufgebaut."));
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
    manualDisconnect_ = true;
    reconnectTimer_.stop();
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

qint64 ElectrumClient::requestHeaderSubscription() {
    return sendRequest(QStringLiteral("blockchain.headers.subscribe"), QJsonArray{});
}

void ElectrumClient::setAutoReconnect(bool enabled) { autoReconnect_ = enabled; }

void ElectrumClient::onConnected() {
    reconnectAttempt_ = 0;
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
                QJsonArray{QStringLiteral("BC2-Cold-Wallet-Simulator/0.22.0"),
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

    pending_.insert(requestId, PendingRequest{method, scriptHash, clock_.elapsed()});
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
    if (requestId < 0) {
        if (object.value(QStringLiteral("method")).toString() == QStringLiteral("blockchain.headers.subscribe")) {
            const QJsonArray params = object.value(QStringLiteral("params")).toArray();
            if (!params.isEmpty() && params.first().isObject()) {
                emit blockHeightChanged(params.first().toObject().value(QStringLiteral("height")).toInt());
            }
        }
        return;
    }
    if (!pending_.contains(requestId)) return;

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
        requestHeaderSubscription();
        return;
    }
    if (pending.method == QStringLiteral("blockchain.headers.subscribe") && result.isObject()) {
        emit blockHeightChanged(result.toObject().value(QStringLiteral("height")).toInt());
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
    scheduleReconnect();
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
    scheduleReconnect();
}

void ElectrumClient::checkRequestTimeouts() {
    const qint64 now = clock_.elapsed();
    QList<qint64> timedOut;
    for (auto it = pending_.cbegin(); it != pending_.cend(); ++it) {
        if (now - it.value().startedAtMs >= kRequestTimeoutMs) timedOut.append(it.key());
    }
    for (qint64 id : timedOut) {
        const PendingRequest request = pending_.take(id);
        emit requestFailed(id, request.method, request.scriptHash, QStringLiteral("Electrum-Anfrage hat das Zeitlimit überschritten."));
    }
}

void ElectrumClient::scheduleReconnect() {
    if (!autoReconnect_ || manualDisconnect_ || host_.isEmpty() || port_ == 0 || reconnectTimer_.isActive()) return;
    const int delay = qMin(1000 * (1 << qMin(reconnectAttempt_, 5)), kMaximumReconnectDelayMs);
    ++reconnectAttempt_;
    emit stateChanged(QStringLiteral("Offline · neuer Versuch in %1 s").arg(delay / 1000), false);
    reconnectTimer_.start(delay);
}

void ElectrumClient::reconnect() {
    if (manualDisconnect_) return;
    emit stateChanged(QStringLiteral("Automatische Wiederverbindung …"), false);
    if (useSsl_) socket_.connectToHostEncrypted(host_, port_); else socket_.connectToHost(host_, port_);
}

void ElectrumClient::failPendingRequests(const QString &message) {
    const auto requests = pending_;
    pending_.clear();
    for (auto it = requests.cbegin(); it != requests.cend(); ++it) {
        emit requestFailed(it.key(), it.value().method, it.value().scriptHash, message);
    }
}
