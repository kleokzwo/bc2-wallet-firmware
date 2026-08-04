#include "hardwarewalletclient.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>
#include <QThread>

#include <algorithm>

namespace {
constexpr char kMagic[] = {'B', 'C', '2'};
constexpr quint8 kProtocolVersion = 1U;
constexpr quint8 kPingCommand = 0x01U;
constexpr quint8 kReviewReceiveCommand = 0x20U;
constexpr quint8 kReviewTransactionCommand = 0x21U;
constexpr quint8 kGetTransactionResultCommand = 0x22U;
constexpr quint8 kResponseFlag = 0x80U;
constexpr int kHeaderSize = 9;
constexpr int kMaximumPayload = 512;
constexpr int kTransactionPollIntervalMs = 500;

bool serialDeviceDisconnected(const QSerialPort &port) {
    return !port.isOpen() ||
           port.error() == QSerialPort::DeviceNotFoundError ||
           port.error() == QSerialPort::PermissionError ||
           port.error() == QSerialPort::ResourceError;
}

QByteArray encodeFrame(quint8 command, quint16 sequence, const QByteArray &payload) {
    QByteArray frame(kMagic, 3);
    frame.append(static_cast<char>(kProtocolVersion));
    frame.append(static_cast<char>(command));
    frame.append(static_cast<char>(sequence & 0xffU));
    frame.append(static_cast<char>((sequence >> 8U) & 0xffU));
    frame.append(static_cast<char>(payload.size() & 0xff));
    frame.append(static_cast<char>((payload.size() >> 8) & 0xff));
    frame.append(payload);
    return frame;
}

bool readFrame(QSerialPort &port, quint8 expectedCommand, quint16 expectedSequence,
               QByteArray *payload) {
    QByteArray buffer;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 2000) {
        if (port.bytesAvailable() == 0 && !port.waitForReadyRead(100)) continue;
        buffer.append(port.readAll());
        const int magicAt = buffer.indexOf(QByteArray(kMagic, 3));
        if (magicAt < 0) {
            if (buffer.size() > 2) buffer = buffer.right(2);
            continue;
        }
        if (magicAt > 0) buffer.remove(0, magicAt);
        if (buffer.size() < kHeaderSize) continue;
        const auto byte = [&buffer](int index) { return static_cast<quint8>(buffer.at(index)); };
        const int size = byte(7) | (static_cast<int>(byte(8)) << 8);
        if (byte(3) != kProtocolVersion || size > kMaximumPayload) {
            buffer.remove(0, 1);
            continue;
        }
        if (buffer.size() < kHeaderSize + size) continue;
        const quint16 sequence = byte(5) | (static_cast<quint16>(byte(6)) << 8U);
        if (byte(4) != expectedCommand || sequence != expectedSequence) {
            buffer.remove(0, kHeaderSize + size);
            continue;
        }
        *payload = buffer.mid(kHeaderSize, size);
        return true;
    }
    return false;
}

bool request(QSerialPort &port, quint8 command, quint16 sequence,
             const QByteArray &requestPayload, QByteArray *responsePayload) {
    const QByteArray frame = encodeFrame(command, sequence, requestPayload);
    if (port.write(frame) != frame.size() || !port.waitForBytesWritten(1000)) return false;
    return readFrame(port, static_cast<quint8>(command | kResponseFlag), sequence,
                     responsePayload);
}
}

HardwareWalletClient::HardwareWalletClient(QObject *parent) : QObject(parent) {}

HardwareWalletClient::Result HardwareWalletClient::reviewReceiveAddress(const QString &address) {
    const QByteArray encodedAddress = address.toLatin1();
    if (encodedAddress.isEmpty() || encodedAddress.size() > kMaximumPayload) {
        return {false, {}, QStringLiteral("Die Empfangsadresse ist ungültig.")};
    }
    return reviewPayload(kReviewReceiveCommand, encodedAddress,
                         QStringLiteral("Adresse übertragen · bitte jetzt am Gerät mit PIN freigeben"));
}

HardwareWalletClient::Result HardwareWalletClient::reviewTransaction(
    const QString &address, quint64 amount, quint64 change, quint64 fee) {
    const QByteArray encodedAddress = address.toLatin1();
    if (encodedAddress.isEmpty() || encodedAddress.size() >= 96 || amount == 0U)
        return {false, {}, QStringLiteral("Die Transaktionsdaten sind ungültig.")};
    QByteArray payload;
    payload.reserve(28 + encodedAddress.size());
    payload.append(char(1));
    payload.append(char(1));
    payload.append(char(1));
    const auto appendU64 = [&payload](quint64 value) {
        for (unsigned int index = 0U; index < 8U; ++index)
            payload.append(static_cast<char>((value >> (index * 8U)) & 0xffU));
    };
    appendU64(amount);
    appendU64(change);
    appendU64(fee);
    payload.append(static_cast<char>(encodedAddress.size()));
    payload.append(encodedAddress);
    return reviewPayload(kReviewTransactionCommand, payload,
                         QStringLiteral("Transaktion auf der Hardware bestätigt."));
}

HardwareWalletClient::Result HardwareWalletClient::reviewPayload(
    int command, const QByteArray &payload, const QString &acceptedMessage) {
    if (command < 0 || command > 255 || payload.size() > kMaximumPayload)
        return {false, {}, QStringLiteral("Die Geräteanfrage ist ungültig.")};

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    std::stable_sort(ports.begin(), ports.end(), [](const QSerialPortInfo &left,
                                                    const QSerialPortInfo &right) {
        const auto isLikelyUsbPort = [](const QSerialPortInfo &port) {
            const QString name = port.portName();
            return name.startsWith(QStringLiteral("ttyACM"), Qt::CaseInsensitive) ||
                   name.startsWith(QStringLiteral("ttyUSB"), Qt::CaseInsensitive) ||
                   name.startsWith(QStringLiteral("cu.usb"), Qt::CaseInsensitive) ||
                   name.startsWith(QStringLiteral("COM"), Qt::CaseInsensitive);
        };
        return isLikelyUsbPort(left) && !isLikelyUsbPort(right);
    });

    QStringList openedPorts;
    QStringList unavailablePorts;
    QStringList silentPorts;
    for (const QSerialPortInfo &info : ports) {
        QSerialPort port(info);
        port.setBaudRate(QSerialPort::Baud115200);
        if (!port.open(QIODevice::ReadWrite)) {
            unavailablePorts.append(info.systemLocation());
            continue;
        }
        openedPorts.append(info.systemLocation());

        QByteArray ping;
        bool deviceFound = false;
        for (int attempt = 0; attempt < 3 && !deviceFound; ++attempt) {
            if (attempt > 0) QThread::msleep(150);
            deviceFound = request(port, kPingCommand, static_cast<quint16>(attempt + 1),
                                  QByteArrayLiteral("simulator"), &ping) &&
                          ping == QByteArrayLiteral("simulator");
        }
        if (!deviceFound) {
            silentPorts.append(info.systemLocation());
            continue;
        }

        QByteArray response;
        if (!request(port, static_cast<quint8>(command), 2U, payload, &response) ||
            response.size() != 1) {
            return {false, info.portName(), QStringLiteral("Das BC2-Gerät antwortet nicht korrekt.")};
        }
        switch (static_cast<quint8>(response.at(0))) {
            case 1U:
                if (command != kReviewTransactionCommand)
                    return {true, info.portName(), acceptedMessage};
                break;
            case 2U:
                return {false, info.portName(),
                        QStringLiteral("Gerät zuerst entsperren und im Dashboard lassen.")};
            case 3U:
                return {false, info.portName(),
                        QStringLiteral("Auf der Hardware läuft bereits eine Transaktionsprüfung.")};
            default:
                return {false, info.portName(),
                        QStringLiteral("Die Hardware hat ungültige Transaktionsdaten zurückgewiesen.")};
        }
        if (command == kReviewTransactionCommand) {
            quint16 pollSequence = 3U;
            for (;;) {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(kTransactionPollIntervalMs);
                QByteArray decision;
                if (!request(port, kGetTransactionResultCommand, pollSequence++, {}, &decision) ||
                    decision.size() != 1) {
                    if (serialDeviceDisconnected(port)) {
                        return {false, info.portName(),
                                QStringLiteral("Verbindung zum BC2-Gerät während der Bestätigung verloren.")};
                    }
                    // The E-paper driver currently blocks the firmware loop while a
                    // screen is refreshed. Keep polling: terminal decisions are
                    // idempotent on the device and therefore cannot be lost between
                    // retries. A physical disconnect is handled above.
                    continue;
                }
                switch (static_cast<quint8>(decision.at(0))) {
                    case 0U:
                        continue;
                    case 1U:
                        return {true, info.portName(), acceptedMessage};
                    case 2U:
                        return {false, info.portName(),
                                QStringLiteral("Transaktion wurde auf der Hardware abgelehnt.")};
                    default:
                        return {false, info.portName(),
                                QStringLiteral("Die Hardwareprüfung ist nicht mehr aktiv.")};
                }
            }
        }
    }
    if (!unavailablePorts.isEmpty() && openedPorts.isEmpty()) {
        return {false, {},
                QStringLiteral("Serieller Anschluss %1 ist belegt oder nicht zugreifbar. "
                               "Seriellen Monitor schließen und erneut versuchen.")
                    .arg(unavailablePorts.join(QStringLiteral(", ")))};
    }
    if (!silentPorts.isEmpty()) {
        return {false, {},
                QStringLiteral("Anschluss %1 wurde gefunden, aber kein BC2-Gerät antwortete. "
                               "Gerät angeschlossen lassen und erneut versuchen.")
                    .arg(silentPorts.join(QStringLiteral(", ")))};
    }
    return {false, {}, QStringLiteral("Kein serielles USB-Gerät gefunden.")};
}
