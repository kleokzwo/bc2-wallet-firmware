#include "bc2_device_service.h"

#include "bc2_usb_protocol.h"

#include <stdio.h>
#include <string.h>

#define BC2_USB_RESPONSE_FLAG 0x80U

static size_t build_info_payload(const bc2_device_identity_t *identity,
                                 char *output,
                                 size_t output_capacity) {
    if (identity == NULL || output == NULL || output_capacity == 0U) return 0U;

    const int written = snprintf(output,
                                 output_capacity,
                                 "BC2 Cold Wallet %s\n%s\n%ux%u",
                                 BC2_DEVICE_FIRMWARE_VERSION,
                                 identity->board_name != NULL ? identity->board_name : "Unknown board",
                                 (unsigned int)identity->display_width,
                                 (unsigned int)identity->display_height);
    if (written < 0 || (size_t)written >= output_capacity) return 0U;
    return (size_t)written;
}

static size_t build_response(const bc2_usb_message_t *request,
                             const bc2_device_machine *machine,
                             const bc2_device_identity_t *identity,
                             uint8_t *output,
                             size_t output_capacity) {
    uint8_t payload[BC2_DEVICE_INFO_MAX];
    size_t payload_size = 0U;

    switch ((bc2_usb_command_t)request->command) {
        case BC2_USB_CMD_PING:
            if (request->payload_length > sizeof(payload)) return 0U;
            memcpy(payload, request->payload, request->payload_length);
            payload_size = request->payload_length;
            break;
        case BC2_USB_CMD_GET_INFO:
            payload_size = build_info_payload(identity, (char *)payload, sizeof(payload));
            break;
        case BC2_USB_CMD_GET_STATE:
            if (machine == NULL) return 0U;
            payload[0] = (uint8_t)machine->state;
            payload_size = 1U;
            break;
        default:
            return 0U;
    }

    return bc2_usb_encode((uint8_t)(request->command | BC2_USB_RESPONSE_FLAG),
                          request->sequence,
                          payload,
                          (uint16_t)payload_size,
                          output,
                          output_capacity);
}

bc2_hal_result_t bc2_device_service_process_usb(const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity) {
    uint8_t request_buffer[BC2_HAL_USB_MAX_MESSAGE];
    uint8_t response_buffer[BC2_HAL_USB_MAX_MESSAGE];
    size_t request_size = 0U;
    bc2_usb_message_t request;

    const bc2_hal_result_t receive_result = bc2_hal_usb_receive(hal,
                                                               request_buffer,
                                                               sizeof(request_buffer),
                                                               &request_size);
    if (receive_result != BC2_HAL_OK) return receive_result;
    if (request_size == 0U) return BC2_HAL_ERROR_NOT_FOUND;
    if (bc2_usb_parse(request_buffer, request_size, &request) != BC2_USB_PARSE_OK)
        return BC2_HAL_ERROR_IO;

    const size_t response_size = build_response(&request,
                                                machine,
                                                identity,
                                                response_buffer,
                                                sizeof(response_buffer));
    if (response_size == 0U) return BC2_HAL_ERROR_UNAVAILABLE;
    return bc2_hal_usb_send(hal, response_buffer, response_size);
}
