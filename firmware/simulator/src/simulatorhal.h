#ifndef BC2_SIMULATOR_HAL_H
#define BC2_SIMULATOR_HAL_H

#include <QByteArray>
#include <QHash>
#include <QQueue>

extern "C" {
#include "bc2_hal.h"
}

class EpaperWidget;

class SimulatorHal final {
public:
    explicit SimulatorHal(EpaperWidget *display);
    const bc2_hal_t *interface() const;
    void enqueueButton(bc2_button_t button, bc2_button_action_t action);
    QByteArray lastUsbMessage() const;

private:
    static bc2_hal_result_t displayPresent(void *context, const bc2_display_frame_t *frame);
    static bc2_hal_result_t buttonPoll(void *context, bc2_button_event_t *event);
    static uint64_t timeNowMs(void *context);
    static bc2_hal_result_t randomFill(void *context, uint8_t *output, size_t outputSize);
    static bc2_hal_result_t storageRead(void *context, const char *key, uint8_t *output,
                                        size_t outputCapacity, size_t *outputSize);
    static bc2_hal_result_t storageWrite(void *context, const char *key, const uint8_t *data,
                                         size_t dataSize);
    static bc2_hal_result_t storageRemove(void *context, const char *key);
    static bc2_hal_result_t usbSend(void *context, const uint8_t *data, size_t dataSize);
    static bc2_hal_result_t usbReceive(void *context, uint8_t *output, size_t outputCapacity,
                                       size_t *outputSize);

    EpaperWidget *display_;
    bc2_hal_t hal_{};
    QHash<QString, QByteArray> storage_;
    QQueue<bc2_button_event_t> buttons_;
    QByteArray usbMessage_;
};

#endif
