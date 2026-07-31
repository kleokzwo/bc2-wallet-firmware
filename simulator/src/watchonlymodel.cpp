#include "watchonlymodel.h"

extern "C" {
#include "bc2_crypto.h"
#include "bc2_wallet.h"
}

#include <QByteArray>
#include <QSet>
#include <algorithm>

namespace {
constexpr const char *kTestMnemonic =
    "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";

QByteArray scriptForPublicKey(const uint8_t publicKey[33]) {
    uint8_t keyHash[20] = {0};
    if (!bc2_hash160(publicKey, 33U, keyHash)) return {};
    QByteArray script(22, Qt::Uninitialized);
    script[0] = 0x00; script[1] = 0x14;
    for (int i = 0; i < 20; ++i) script[2 + i] = static_cast<char>(keyHash[i]);
    return script;
}

QString scriptHashForScript(const QByteArray &scriptBytes) {
    uint8_t digest[32] = {0};
    if (scriptBytes.size() != 22 || !bc2_sha256(reinterpret_cast<const uint8_t *>(scriptBytes.constData()), static_cast<size_t>(scriptBytes.size()), digest)) {
        return {};
    }
    QByteArray reversed(32, Qt::Uninitialized);
    for (int i = 0; i < 32; ++i) {
        reversed[i] = static_cast<char>(digest[31 - i]);
    }
    return QString::fromLatin1(reversed.toHex());
}
}

WatchOnlyModel::WatchOnlyModel(QObject *parent) : QObject(parent) {}

bool WatchOnlyModel::buildDemoAccount(unsigned int receiveCount, unsigned int changeCount) {
    addresses_.clear();
    accounts_.clear();
    accounts_.push_back({QStringLiteral("account-0"), QStringLiteral("BC2 Hauptkonto"), QStringLiteral("Vorbereitet für XPUB-Import")});
    for (unsigned int change = 0; change < 2; ++change) {
        const unsigned int count = change == 0 ? receiveCount : changeCount;
        for (unsigned int index = 0; index < count; ++index) {
            bc2_receive_address result{};
            if (bc2_wallet_receive_address_from_mnemonic(kTestMnemonic, "", bc2_network_mainnet(),
                                                         0U, change, index, &result) != BC2_WALLET_OK) {
                addresses_.clear();
                return false;
            }
            const QByteArray scriptPubKey = scriptForPublicKey(result.public_key);
            const QString scriptHash = scriptHashForScript(scriptPubKey);
            if (scriptHash.isEmpty() || scriptPubKey.isEmpty()) {
                addresses_.clear();
                return false;
            }
            addresses_.push_back(WatchAddress{QString::fromLatin1(result.address),
                                              QString::fromLatin1(result.path), scriptHash, scriptPubKey,
                                              change != 0, index, 0, 0, {}, {}});
        }
    }
    emit changed();
    return true;
}

const QVector<WatchAddress> &WatchOnlyModel::addresses() const { return addresses_; }
const QVector<WatchAccount> &WatchOnlyModel::accounts() const { return accounts_; }

QVector<WatchTransaction> WatchOnlyModel::transactions() const {
    QHash<QString, WatchTransaction> items;
    for (const WatchAddress &address : addresses_) {
        for (const WatchHistoryItem &history : address.history) {
            auto item = items.value(history.txHash, WatchTransaction{history.txHash, history.height, 0});
            item.height = qMax(item.height, history.height);
            for (const WatchUtxo &utxo : address.utxos) {
                if (utxo.txHash == history.txHash) item.knownAmount += utxo.value;
            }
            items.insert(history.txHash, item);
        }
    }
    QVector<WatchTransaction> result;
    result.reserve(items.size());
    for (auto it = items.cbegin(); it != items.cend(); ++it) result.push_back(it.value());
    std::sort(result.begin(), result.end(), [](const WatchTransaction &a, const WatchTransaction &b) { return a.height > b.height; });
    return result;
}

qint64 WatchOnlyModel::totalConfirmed() const {
    qint64 total = 0;
    for (const WatchAddress &address : addresses_) {
        total += address.confirmed;
    }
    return total;
}

qint64 WatchOnlyModel::totalUnconfirmed() const {
    qint64 total = 0;
    for (const WatchAddress &address : addresses_) {
        total += address.unconfirmed;
    }
    return total;
}

int WatchOnlyModel::transactionCount() const {
    QSet<QString> hashes;
    for (const WatchAddress &address : addresses_) {
        for (const WatchHistoryItem &item : address.history) {
            hashes.insert(item.txHash);
        }
    }
    return hashes.size();
}

int WatchOnlyModel::utxoCount() const {
    int count = 0;
    for (const WatchAddress &address : addresses_) {
        count += address.utxos.size();
    }
    return count;
}

bool WatchOnlyModel::setBalance(const QString &scriptHash, qint64 confirmed, qint64 unconfirmed) {
    WatchAddress *address = find(scriptHash);
    if (address == nullptr) {
        return false;
    }
    address->confirmed = confirmed;
    address->unconfirmed = unconfirmed;
    emit changed();
    return true;
}

bool WatchOnlyModel::setHistory(const QString &scriptHash, const QVector<WatchHistoryItem> &history) {
    WatchAddress *address = find(scriptHash);
    if (address == nullptr) {
        return false;
    }
    address->history = history;
    emit changed();
    return true;
}

bool WatchOnlyModel::setUtxos(const QString &scriptHash, const QVector<WatchUtxo> &utxos) {
    WatchAddress *address = find(scriptHash);
    if (address == nullptr) {
        return false;
    }
    address->utxos = utxos;
    emit changed();
    return true;
}

void WatchOnlyModel::clearNetworkData() {
    for (WatchAddress &address : addresses_) {
        address.confirmed = 0;
        address.unconfirmed = 0;
        address.utxos.clear();
        address.history.clear();
    }
    emit changed();
}

WatchAddress *WatchOnlyModel::find(const QString &scriptHash) {
    for (WatchAddress &address : addresses_) {
        if (address.scriptHash == scriptHash) {
            return &address;
        }
    }
    return nullptr;
}
