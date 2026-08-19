#ifndef BC2_USB_PROTOCOL_H
#define BC2_USB_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define BC2_USB_PROTOCOL_VERSION 1U
#define BC2_USB_MAGIC_0 0x42U
#define BC2_USB_MAGIC_1 0x43U
#define BC2_USB_MAGIC_2 0x32U
#define BC2_USB_HEADER_SIZE 9U
#define BC2_USB_PAYLOAD_MAX 512U
typedef enum {
    BC2_USB_CMD_PING = 0x01,
    BC2_USB_CMD_GET_INFO = 0x02,
    BC2_USB_CMD_GET_STATE = 0x03,
    BC2_USB_CMD_GET_CAPABILITIES = 0x04,
    BC2_USB_CMD_GET_WALLET_STATUS = 0x40,
    BC2_USB_CMD_BEGIN_CREATE_WALLET = 0x41,
    BC2_USB_CMD_BEGIN_RECEIVE_ADDRESS = 0x42,
    BC2_USB_CMD_GET_RECEIVE_RESULT = 0x43,
    BC2_USB_CMD_BEGIN_RECOVERY = 0x44,
    BC2_USB_CMD_BEGIN_UNLOCK = 0x45,
    BC2_USB_CMD_SUBMIT_RECOVERY_MNEMONIC = 0x46,
    BC2_USB_CMD_LOCK_WALLET = 0x47,
    BC2_USB_CMD_REVIEW_RECEIVE_ADDRESS = 0x20,
    BC2_USB_CMD_REVIEW_TRANSACTION = 0x21,
    BC2_USB_CMD_GET_TRANSACTION_RESULT = 0x22,
    BC2_USB_CMD_SIGN_TRANSACTION = 0x23,
    BC2_USB_CMD_GET_SIGN_RESULT = 0x24,
    BC2_USB_CMD_DISPLAY_TEST = 0x10,
    BC2_USB_CMD_BUTTON_TEST = 0x11
} bc2_usb_command_t;
typedef enum {
    BC2_USB_PARSE_OK = 0,
    BC2_USB_PARSE_ARGUMENT,
    BC2_USB_PARSE_TRUNCATED,
    BC2_USB_PARSE_MAGIC,
    BC2_USB_PARSE_VERSION,
    BC2_USB_PARSE_LIMIT,
    BC2_USB_PARSE_LENGTH
} bc2_usb_parse_result_t;
typedef struct {
    uint8_t command;
    uint16_t sequence;
    uint16_t payload_length;
    const uint8_t *payload;
} bc2_usb_message_t;
size_t bc2_usb_encode(uint8_t command, uint16_t sequence, const uint8_t *payload,
                      uint16_t payload_length, uint8_t *output, size_t output_capacity);
bc2_usb_parse_result_t bc2_usb_parse(const uint8_t *data, size_t data_size,
                                     bc2_usb_message_t *message);
#ifdef __cplusplus
}
#endif
#endif
