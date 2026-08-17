#include "devicecontroller.h"

#include <QDateTime>
#include <QTimer>

DeviceController::DeviceController(QObject *parent)
    : QObject(parent), timer_(new QTimer(this)) {
    bc2_device_machine_init(&machine_, 1, nowMs());
    machine_.session_timeout_ms = 5ULL * 60ULL * 1000ULL;
    timer_->setInterval(250);
    connect(timer_, &QTimer::timeout, this, &DeviceController::tick);
    timer_->start();
}

bc2_device_state DeviceController::state() const { return machine_.state; }
QString DeviceController::stateText() const { return QString::fromLatin1(bc2_device_state_name(machine_.state)); }
bool DeviceController::isUnlocked() const { return bc2_device_machine_is_unlocked(&machine_) != 0; }
unsigned int DeviceController::failedAttempts() const { return machine_.failed_unlock_attempts; }
unsigned int DeviceController::maximumAttempts() const { return machine_.max_unlock_attempts; }

QString DeviceController::cooldownText() const {
    const quint64 remaining = bc2_device_machine_cooldown_remaining(&machine_, nowMs());
    if (remaining == 0U) return QString();
    const quint64 seconds = (remaining + 999U) / 1000U;
    return QStringLiteral("Erneuter Versuch in %1 Sekunden").arg(seconds);
}

void DeviceController::boot() { dispatch(BC2_DEVICE_EVENT_BOOT_COMPLETE); }
void DeviceController::beginUnlock() { dispatch(BC2_DEVICE_EVENT_BEGIN_UNLOCK); }
void DeviceController::submitPinResult(bool accepted) {
    dispatch(accepted ? BC2_DEVICE_EVENT_UNLOCK_SUCCESS : BC2_DEVICE_EVENT_UNLOCK_FAILURE);
}
void DeviceController::cancel() { dispatch(BC2_DEVICE_EVENT_CANCEL); }
void DeviceController::lock() { dispatch(BC2_DEVICE_EVENT_LOCK); }
void DeviceController::noteActivity() { dispatch(BC2_DEVICE_EVENT_ACTIVITY); }
void DeviceController::openReceiveReview() { dispatch(BC2_DEVICE_EVENT_OPEN_RECEIVE); }
void DeviceController::openTransactionReview() { dispatch(BC2_DEVICE_EVENT_OPEN_TRANSACTION); }
void DeviceController::openSettings() { dispatch(BC2_DEVICE_EVENT_OPEN_SETTINGS); }
void DeviceController::confirm() { dispatch(BC2_DEVICE_EVENT_CONFIRM); }

void DeviceController::handleButton(bc2_button_t button, bc2_button_action_t action) {
    bc2_button_event_t buttonEvent{button, action, nowMs()};
    bc2_device_event deviceEvent;
    if (bc2_device_flow_event_from_button(machine_.state, &buttonEvent, &deviceEvent) != 0) {
        dispatch(deviceEvent);
    }
}

quint64 DeviceController::nowMs() { return static_cast<quint64>(QDateTime::currentMSecsSinceEpoch()); }

void DeviceController::dispatch(bc2_device_event event) {
    const bc2_device_state previousState = machine_.state;
    const bc2_device_action previousAction = machine_.last_action;
    if (bc2_device_machine_dispatch(&machine_, event, nowMs()) != 0) publishChanges(previousState, previousAction);
}

void DeviceController::tick() {
    const bc2_device_state previousState = machine_.state;
    const bc2_device_action previousAction = machine_.last_action;
    if (bc2_device_machine_tick(&machine_, nowMs()) != 0 || machine_.state == BC2_DEVICE_COOLDOWN) {
        publishChanges(previousState, previousAction);
    }
}

void DeviceController::publishChanges(bc2_device_state previousState, bc2_device_action previousAction) {
    emit changed();
    if (machine_.state == BC2_DEVICE_LOCKED && previousState != BC2_DEVICE_LOCKED) emit locked();
    if (machine_.state == BC2_DEVICE_DASHBOARD && previousState != BC2_DEVICE_DASHBOARD &&
        machine_.last_action == BC2_DEVICE_ACTION_UNLOCKED) emit unlocked();
    if (machine_.last_action != BC2_DEVICE_ACTION_NONE && machine_.last_action != previousAction) {
        emit actionCompleted(QString::fromLatin1(bc2_device_action_name(machine_.last_action)));
    }
}
