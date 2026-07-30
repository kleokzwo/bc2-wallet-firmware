#include "simulatorhal.h"
#include "epaperwidget.h"

#include <QDateTime>
#include <QRandomGenerator>
#include <QString>
#include <cstring>

SimulatorHal::SimulatorHal(EpaperWidget *display) : display_(display) {
    hal_.context = this;
    hal_.display_present = &SimulatorHal::displayPresent;
    hal_.button_poll = &SimulatorHal::buttonPoll;
    hal_.time_now_ms = &SimulatorHal::timeNowMs;
    hal_.random_fill = &SimulatorHal::randomFill;
    hal_.storage_read = &SimulatorHal::storageRead;
    hal_.storage_write = &SimulatorHal::storageWrite;
    hal_.storage_remove = &SimulatorHal::storageRemove;
    hal_.usb_send = &SimulatorHal::usbSend;
    hal_.usb_receive = &SimulatorHal::usbReceive;
}

const bc2_hal_t *SimulatorHal::interface() const { return &hal_; }

void SimulatorHal::enqueueButton(bc2_button_t button, bc2_button_action_t action) {
    buttons_.enqueue({button, action, timeNowMs(this)});
}

QByteArray SimulatorHal::lastUsbMessage() const { return usbMessage_; }

bc2_hal_result_t SimulatorHal::displayPresent(void *context, const bc2_display_frame_t *frame) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || self->display_ == nullptr || frame == nullptr) return BC2_HAL_ERROR_ARGUMENT;
    self->display_->setTitle(QString::fromUtf8(frame->title));
    self->display_->setBody(QString::fromUtf8(frame->body));
    self->display_->setFooter(QString::fromUtf8(frame->footer));
    self->display_->setFullRefreshIndicator(frame->require_full_refresh);
    return BC2_HAL_OK;
}

bc2_hal_result_t SimulatorHal::buttonPoll(void *context, bc2_button_event_t *event) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || event == nullptr) return BC2_HAL_ERROR_ARGUMENT;
    if (self->buttons_.isEmpty()) {
        *event = {BC2_BUTTON_NONE, BC2_BUTTON_RELEASED, timeNowMs(context)};
        return BC2_HAL_ERROR_NOT_FOUND;
    }
    *event = self->buttons_.dequeue();
    return BC2_HAL_OK;
}

uint64_t SimulatorHal::timeNowMs(void *context) {
    Q_UNUSED(context);
    return static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());
}

bc2_hal_result_t SimulatorHal::randomFill(void *context, uint8_t *output, size_t outputSize) {
    Q_UNUSED(context);
    if (output == nullptr || outputSize == 0U) return BC2_HAL_ERROR_ARGUMENT;
    auto *generator = QRandomGenerator::system();
    for (size_t i = 0; i < outputSize; ++i) output[i] = static_cast<uint8_t>(generator->generate() & 0xffU);
    return BC2_HAL_OK;
}

bc2_hal_result_t SimulatorHal::storageRead(void *context, const char *key, uint8_t *output,
                                           size_t outputCapacity, size_t *outputSize) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || key == nullptr || output == nullptr || outputSize == nullptr)
        return BC2_HAL_ERROR_ARGUMENT;
    const auto it = self->storage_.constFind(QString::fromUtf8(key));
    if (it == self->storage_.cend()) return BC2_HAL_ERROR_NOT_FOUND;
    if (outputCapacity < static_cast<size_t>(it->size())) return BC2_HAL_ERROR_LIMIT;
    std::memcpy(output, it->constData(), static_cast<size_t>(it->size()));
    *outputSize = static_cast<size_t>(it->size());
    return BC2_HAL_OK;
}

bc2_hal_result_t SimulatorHal::storageWrite(void *context, const char *key, const uint8_t *data,
                                            size_t dataSize) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || key == nullptr || data == nullptr || dataSize == 0U)
        return BC2_HAL_ERROR_ARGUMENT;
    self->storage_.insert(QString::fromUtf8(key), QByteArray(reinterpret_cast<const char *>(data), static_cast<qsizetype>(dataSize)));
    return BC2_HAL_OK;
}

bc2_hal_result_t SimulatorHal::storageRemove(void *context, const char *key) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || key == nullptr) return BC2_HAL_ERROR_ARGUMENT;
    return self->storage_.remove(QString::fromUtf8(key)) > 0 ? BC2_HAL_OK : BC2_HAL_ERROR_NOT_FOUND;
}

bc2_hal_result_t SimulatorHal::usbSend(void *context, const uint8_t *data, size_t dataSize) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || data == nullptr || dataSize == 0U) return BC2_HAL_ERROR_ARGUMENT;
    self->usbMessage_ = QByteArray(reinterpret_cast<const char *>(data), static_cast<qsizetype>(dataSize));
    return BC2_HAL_OK;
}

bc2_hal_result_t SimulatorHal::usbReceive(void *context, uint8_t *output, size_t outputCapacity,
                                          size_t *outputSize) {
    auto *self = static_cast<SimulatorHal *>(context);
    if (self == nullptr || output == nullptr || outputSize == nullptr) return BC2_HAL_ERROR_ARGUMENT;
    if (self->usbMessage_.isEmpty()) return BC2_HAL_ERROR_NOT_FOUND;
    if (outputCapacity < static_cast<size_t>(self->usbMessage_.size())) return BC2_HAL_ERROR_LIMIT;
    std::memcpy(output, self->usbMessage_.constData(), static_cast<size_t>(self->usbMessage_.size()));
    *outputSize = static_cast<size_t>(self->usbMessage_.size());
    self->usbMessage_.clear();
    return BC2_HAL_OK;
}
