#include "bc2_usb_protocol.h"
size_t bc2_usb_encode(uint8_t command, uint16_t sequence, const uint8_t *payload,
                      uint16_t payload_length, uint8_t *output, size_t output_capacity) {
    const size_t total = BC2_USB_HEADER_SIZE + (size_t)payload_length;
    if (output == NULL || payload_length > BC2_USB_PAYLOAD_MAX || output_capacity < total ||
        (payload_length > 0U && payload == NULL)) return 0U;
    output[0]=BC2_USB_MAGIC_0; output[1]=BC2_USB_MAGIC_1; output[2]=BC2_USB_MAGIC_2;
    output[3]=BC2_USB_PROTOCOL_VERSION; output[4]=command;
    output[5]=(uint8_t)(sequence & 0xffU); output[6]=(uint8_t)(sequence >> 8U);
    output[7]=(uint8_t)(payload_length & 0xffU); output[8]=(uint8_t)(payload_length >> 8U);
    for (uint16_t i=0; i<payload_length; ++i) output[BC2_USB_HEADER_SIZE+i]=payload[i];
    return total;
}
bc2_usb_parse_result_t bc2_usb_parse(const uint8_t *data, size_t data_size,
                                     bc2_usb_message_t *message) {
    if (data == NULL || message == NULL) return BC2_USB_PARSE_ARGUMENT;
    if (data_size < BC2_USB_HEADER_SIZE) return BC2_USB_PARSE_TRUNCATED;
    if (data[0]!=BC2_USB_MAGIC_0 || data[1]!=BC2_USB_MAGIC_1 || data[2]!=BC2_USB_MAGIC_2)
        return BC2_USB_PARSE_MAGIC;
    if (data[3]!=BC2_USB_PROTOCOL_VERSION) return BC2_USB_PARSE_VERSION;
    const uint16_t length=(uint16_t)data[7]|((uint16_t)data[8]<<8U);
    if (length > BC2_USB_PAYLOAD_MAX) return BC2_USB_PARSE_LIMIT;
    if (data_size != BC2_USB_HEADER_SIZE + (size_t)length) return BC2_USB_PARSE_LENGTH;
    message->command=data[4]; message->sequence=(uint16_t)data[5]|((uint16_t)data[6]<<8U);
    message->payload_length=length; message->payload=data+BC2_USB_HEADER_SIZE;
    return BC2_USB_PARSE_OK;
}
