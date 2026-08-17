#include "watchonlysync.h"

#include "electrumclient.h"
#include "watchonlymodel.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

WatchOnlySync::WatchOnlySync(ElectrumClient *client, WatchOnlyModel *model, QObject *parent)
    : QObject(parent), client_(client), model_(model) {
    connect(client_, &ElectrumClient::responseReceived, this, &WatchOnlySync::handleResponse);
    connect(client_, &ElectrumClient::requestFailed, this, &WatchOnlySync::handleFailure);
}

bool WatchOnlySync::isRunning() const { return running_; }

void WatchOnlySync::start() {
    if (running_) {
        return;
    }
    if (!client_->isConnected()) {
        emit finished(false, QStringLiteral("Keine aktive Electrum-Verbindung."));
        return;
    }
    if (model_->addresses().isEmpty()) {
        emit finished(false, QStringLiteral("Keine Watch-only-Adressen vorhanden."));
        return;
    }

    pendingIds_.clear();
    completed_ = 0;
    failures_ = 0;
    running_ = true;
    model_->clearNetworkData();
    total_ = static_cast<int>(model_->addresses().size()) * 3;

    for (const WatchAddress &address : model_->addresses()) {
        const qint64 balanceId = client_->requestBalance(address.scriptHash);
        const qint64 historyId = client_->requestHistory(address.scriptHash);
        const qint64 unspentId = client_->requestUnspent(address.scriptHash);
        if (balanceId >= 0) pendingIds_.insert(balanceId); else ++failures_;
        if (historyId >= 0) pendingIds_.insert(historyId); else ++failures_;
        if (unspentId >= 0) pendingIds_.insert(unspentId); else ++failures_;
    }

    completed_ = total_ - pendingIds_.size();
    emit progressChanged(completed_, total_, QStringLiteral("Öffentliche BC2-Daten werden synchronisiert …"));
    finishIfDone();
}

void WatchOnlySync::cancel() {
    if (!running_) {
        return;
    }
    running_ = false;
    pendingIds_.clear();
    emit finished(false, QStringLiteral("Synchronisation abgebrochen."));
}

void WatchOnlySync::handleResponse(qint64 requestId, const QString &method,
                                   const QString &scriptHash, const QJsonValue &result) {
    if (!running_ || !pendingIds_.contains(requestId)) {
        return;
    }

    bool accepted = false;
    if (method == QStringLiteral("blockchain.scripthash.get_balance") && result.isObject()) {
        const QJsonObject object = result.toObject();
        accepted = model_->setBalance(scriptHash,
                                      object.value(QStringLiteral("confirmed")).toInteger(),
                                      object.value(QStringLiteral("unconfirmed")).toInteger());
    } else if (method == QStringLiteral("blockchain.scripthash.get_history") && result.isArray()) {
        QVector<WatchHistoryItem> history;
        const QJsonArray items = result.toArray();
        history.reserve(items.size());
        for (const QJsonValue &value : items) {
            const QJsonObject item = value.toObject();
            const QString txHash = item.value(QStringLiteral("tx_hash")).toString();
            if (!txHash.isEmpty()) {
                history.push_back({txHash, item.value(QStringLiteral("height")).toInt()});
            }
        }
        accepted = model_->setHistory(scriptHash, history);
    } else if (method == QStringLiteral("blockchain.scripthash.listunspent") && result.isArray()) {
        QVector<WatchUtxo> utxos;
        const QJsonArray items = result.toArray();
        utxos.reserve(items.size());
        for (const QJsonValue &value : items) {
            const QJsonObject item = value.toObject();
            const QString txHash = item.value(QStringLiteral("tx_hash")).toString();
            if (!txHash.isEmpty()) {
                utxos.push_back({txHash,
                                 item.value(QStringLiteral("tx_pos")).toInt(),
                                 item.value(QStringLiteral("value")).toInteger(),
                                 item.value(QStringLiteral("height")).toInt()});
            }
        }
        accepted = model_->setUtxos(scriptHash, utxos);
    }

    if (!accepted) {
        ++failures_;
    }
    completeRequest(requestId);
}

void WatchOnlySync::handleFailure(qint64 requestId, const QString &method,
                                  const QString &scriptHash, const QString &message) {
    Q_UNUSED(method)
    Q_UNUSED(scriptHash)
    Q_UNUSED(message)
    if (!running_ || !pendingIds_.contains(requestId)) {
        return;
    }
    ++failures_;
    completeRequest(requestId);
}

void WatchOnlySync::completeRequest(qint64 requestId) {
    pendingIds_.remove(requestId);
    ++completed_;
    emit progressChanged(completed_, total_,
                         QStringLiteral("%1 von %2 Electrum-Abfragen abgeschlossen")
                             .arg(completed_).arg(total_));
    finishIfDone();
}

void WatchOnlySync::finishIfDone() {
    if (!running_ || !pendingIds_.isEmpty()) {
        return;
    }
    running_ = false;
    if (failures_ == 0) {
        emit finished(true, QStringLiteral("Watch-only-Synchronisation vollständig abgeschlossen."));
    } else {
        emit finished(false,
                      QStringLiteral("Synchronisation beendet, aber %1 Abfragen sind fehlgeschlagen.")
                          .arg(failures_));
    }
}
