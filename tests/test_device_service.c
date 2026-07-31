#include "bc2_device_service.h"
#include "bc2_usb_protocol.h"

#include <assert.h>
#include <string.h>

typedef struct {
    uint8_t request[128];
    size_t request_size;
    uint8_t response[256];
    size_t response_size;
} fake_usb_t;

static bc2_hal_result_t unavailable_display(void *context, const bc2_display_frame_t *frame) {
    (void)context;
    (void)frame;
    return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t unavailable_button(void *context, bc2_button_event_t *event) {
    (void)context;
    (void)event;
    return BC2_HAL_ERROR_UNAVAILABLE;
}
static uint64_t fake_time(void *context) { (void)context; return 0U; }
static bc2_hal_result_t unavailable_random(void *context, uint8_t *output, size_t output_size) {
    (void)context; (void)output; (void)output_size; return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t unavailable_read(void *context, const char *key, uint8_t *output,
                                         size_t capacity, size_t *output_size) {
    (void)context; (void)key; (void)output; (void)capacity; (void)output_size;
    return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t unavailable_write(void *context, const char *key, const uint8_t *data,
                                          size_t data_size) {
    (void)context; (void)key; (void)data; (void)data_size; return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t unavailable_remove(void *context, const char *key) {
    (void)context; (void)key; return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t send_usb(void *context, const uint8_t *data, size_t data_size) {
    fake_usb_t *usb = (fake_usb_t *)context;
    assert(data_size <= sizeof(usb->response));
    memcpy(usb->response, data, data_size);
    usb->response_size = data_size;
    return BC2_HAL_OK;
}
static bc2_hal_result_t receive_usb(void *context, uint8_t *output, size_t capacity,
                                    size_t *output_size) {
    fake_usb_t *usb = (fake_usb_t *)context;
    assert(capacity >= usb->request_size);
    memcpy(output, usb->request, usb->request_size);
    *output_size = usb->request_size;
    usb->request_size = 0U;
    return BC2_HAL_OK;
}

static bc2_hal_t make_hal(fake_usb_t *usb) {
    return (bc2_hal_t){usb, unavailable_display, unavailable_button, fake_time,
                       unavailable_random, unavailable_read, unavailable_write,
                       unavailable_remove, send_usb, receive_usb};
}

int main(void) {
    fake_usb_t usb = {0};
    bc2_hal_t hal = make_hal(&usb);
    bc2_device_machine machine;
    bc2_device_identity_t identity = {"Waveshare ESP32-S3", 200U, 200U};
    bc2_device_machine_init(&machine, 1, 0U);

    const uint8_t ping[] = {'o', 'k'};
    usb.request_size = bc2_usb_encode(BC2_USB_CMD_PING, 7U, ping, sizeof(ping),
                                      usb.request, sizeof(usb.request));
    assert(bc2_device_service_process_usb(&hal, &machine, &identity) == BC2_HAL_OK);

    bc2_usb_message_t response = {0};
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.command == (uint8_t)(BC2_USB_CMD_PING | 0x80U));
    assert(response.sequence == 7U);
    assert(response.payload_length == sizeof(ping));
    assert(memcmp(response.payload, ping, sizeof(ping)) == 0);

    usb.request_size = bc2_usb_encode(BC2_USB_CMD_GET_INFO, 8U, NULL, 0U,
                                      usb.request, sizeof(usb.request));
    assert(bc2_device_service_process_usb(&hal, &machine, &identity) == BC2_HAL_OK);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.command == (uint8_t)(BC2_USB_CMD_GET_INFO | 0x80U));
    assert(response.payload_length > 0U);
    assert(strstr((const char *)response.payload, "0.17.4") != NULL);

    usb.request_size = bc2_usb_encode(BC2_USB_CMD_GET_STATE, 9U, NULL, 0U,
                                      usb.request, sizeof(usb.request));
    assert(bc2_device_service_process_usb(&hal, &machine, &identity) == BC2_HAL_OK);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U);
    assert(response.payload[0] == (uint8_t)machine.state);
    return 0;
}
