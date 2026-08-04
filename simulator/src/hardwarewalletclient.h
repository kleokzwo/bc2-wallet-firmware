#ifndef BC2_SIMULATOR_HARDWAREWALLETCLIENT_H
#define BC2_SIMULATOR_HARDWAREWALLETCLIENT_H

#include <QObject>
#include <QByteArray>
#include <QString>

class HardwareWalletClient final : public QObject {
    Q_OBJECT
public:
    struct Result {
        bool accepted = false;
        QString portName;
        QString message;
    };

    explicit HardwareWalletClient(QObject *parent = nullptr);
    Result reviewReceiveAddress(const QString &address);
    Result reviewTransaction(const QString &address, quint64 amount, quint64 change,
                             quint64 fee);
private:
    Result reviewPayload(int command, const QByteArray &payload,
                         const QString &acceptedMessage);
};

#endif
