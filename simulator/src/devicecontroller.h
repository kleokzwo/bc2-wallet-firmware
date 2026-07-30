#ifndef BC2_SIMULATOR_DEVICECONTROLLER_H
#define BC2_SIMULATOR_DEVICECONTROLLER_H

#include <QObject>
#include <QString>

extern "C" {
#include "bc2_device_state.h"
}

class QTimer;

class DeviceController final : public QObject {
    Q_OBJECT
public:
    explicit DeviceController(QObject *parent = nullptr);

    bc2_device_state state() const;
    QString stateText() const;
    QString cooldownText() const;
    bool isUnlocked() const;
    unsigned int failedAttempts() const;
    unsigned int maximumAttempts() const;

public slots:
    void boot();
    void beginUnlock();
    void submitPinResult(bool accepted);
    void cancel();
    void lock();
    void noteActivity();
    void openReceiveReview();
    void openTransactionReview();
    void openSettings();
    void confirm();

signals:
    void changed();
    void locked();
    void unlocked();
    void actionCompleted(const QString &action);

private slots:
    void tick();

private:
    static quint64 nowMs();
    void dispatch(bc2_device_event event);
    void publishChanges(bc2_device_state previousState, bc2_device_action previousAction);

    bc2_device_machine machine_{};
    QTimer *timer_ = nullptr;
};

#endif
