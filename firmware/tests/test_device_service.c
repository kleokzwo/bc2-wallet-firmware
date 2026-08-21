#include "bc2_device_service.h"
#include "bc2_usb_protocol.h"

#include <assert.h>
#include <string.h>

typedef struct {
    uint8_t request[256];
    size_t request_size;
    size_t read_offset;
    size_t read_chunk_size;
    uint8_t response[256];
    size_t response_size;
} fake_usb_t;

static bc2_hal_result_t unavailable_display(void *context, const bc2_display_frame_t *frame) {
    (void)context; (void)frame; return BC2_HAL_ERROR_UNAVAILABLE;
}
static bc2_hal_result_t unavailable_button(void *context, bc2_button_event_t *event) {
    (void)context; (void)event; return BC2_HAL_ERROR_UNAVAILABLE;
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
    const size_t remaining = usb->request_size - usb->read_offset;
    if (remaining == 0U) {
        *output_size = 0U;
        return BC2_HAL_ERROR_NOT_FOUND;
    }
    size_t chunk = remaining;
    if (usb->read_chunk_size > 0U && chunk > usb->read_chunk_size) chunk = usb->read_chunk_size;
    assert(capacity >= chunk);
    memcpy(output, usb->request + usb->read_offset, chunk);
    usb->read_offset += chunk;
    *output_size = chunk;
    return BC2_HAL_OK;
}

static bc2_hal_t make_hal(fake_usb_t *usb) {
    return (bc2_hal_t){usb, unavailable_display, unavailable_button, fake_time,
                       unavailable_random, unavailable_read, unavailable_write,
                       unavailable_remove, send_usb, receive_usb};
}

static void prepare_request(fake_usb_t *usb, uint8_t command, uint16_t sequence,
                            const uint8_t *payload, uint16_t payload_size) {
    memset(usb, 0, sizeof(*usb));
    usb->request_size = bc2_usb_encode(command, sequence, payload, payload_size,
                                       usb->request, sizeof(usb->request));
    assert(usb->request_size > 0U);
}

static void write_u64_le(uint8_t *output, uint64_t value) {
    for (unsigned int index = 0U; index < 8U; ++index)
        output[index] = (uint8_t)(value >> (index * 8U));
}

static void process_until_response(bc2_device_service_t *service, const bc2_hal_t *hal,
                                   const bc2_device_machine *machine,
                                   const bc2_device_identity_t *identity,
                                   fake_usb_t *usb) {
    for (unsigned int i = 0U; i < 32U && usb->response_size == 0U; ++i) {
        const bc2_hal_result_t result = bc2_device_service_process_usb(service, hal, machine, identity);
        assert(result == BC2_HAL_OK || result == BC2_HAL_ERROR_NOT_FOUND);
    }
    assert(usb->response_size > 0U);
}

int main(void) {
    fake_usb_t usb = {0};
    bc2_hal_t hal = make_hal(&usb);
    bc2_device_machine machine;
    bc2_device_service_t service;
    const bc2_device_identity_t identity = {
        "Waveshare ESP32-S3", 200U, 200U, 0U,
        BC2_DEVICE_CAP_USB | BC2_DEVICE_CAP_STORAGE | BC2_DEVICE_CAP_RANDOM
    };
    bc2_device_machine_init(&machine, 1, 0U);
    bc2_device_service_init(&service);

    const uint8_t ping[] = {'o', 'k'};
    prepare_request(&usb, BC2_USB_CMD_PING, 7U, ping, sizeof(ping));
    usb.read_chunk_size = 3U;
    process_until_response(&service, &hal, &machine, &identity, &usb);

    bc2_usb_message_t response = {0};
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.command == (uint8_t)(BC2_USB_CMD_PING | 0x80U));
    assert(response.sequence == 7U);
    assert(response.payload_length == sizeof(ping));
    assert(memcmp(response.payload, ping, sizeof(ping)) == 0);

    prepare_request(&usb, BC2_USB_CMD_GET_INFO, 8U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(strstr((const char *)response.payload, BC2_DEVICE_FIRMWARE_VERSION) != NULL);

    prepare_request(&usb, BC2_USB_CMD_GET_CAPABILITIES, 9U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 2U);
    assert(response.payload[0] == identity.capabilities);
    assert(response.payload[1] == identity.board_revision);

    prepare_request(&usb, BC2_USB_CMD_GET_WALLET_STATUS, 91U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 2U);

    machine.state = BC2_DEVICE_SETUP_REQUIRED;
    machine.wallet_is_initialized = 0;
    prepare_request(&usb, BC2_USB_CMD_GET_WALLET_STATUS, 92U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 0U);

    prepare_request(&usb, BC2_USB_CMD_BEGIN_CREATE_WALLET, 93U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_create_wallet(&service));
    assert(!bc2_device_service_take_create_wallet(&service));

    prepare_request(&usb, BC2_USB_CMD_BEGIN_RECOVERY, 931U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_recovery(&service));
    assert(!bc2_device_service_take_recovery(&service));

    machine.state = BC2_DEVICE_LOCKED;
    machine.wallet_is_initialized = 1;

    prepare_request(&usb, BC2_USB_CMD_BEGIN_CREATE_WALLET, 9315U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_create_wallet(&service));
    assert(!bc2_device_service_take_create_wallet(&service));

    prepare_request(&usb, BC2_USB_CMD_BEGIN_UNLOCK, 932U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_unlock(&service));
    assert(!bc2_device_service_take_unlock(&service));

    machine.state = BC2_DEVICE_DASHBOARD;

    /* Wallet identity is only available after authentication and is supplied
     * by the hardware wallet layer, never by the desktop. */
    uint8_t wallet_id[BC2_DEVICE_WALLET_ID_SIZE];
    for (size_t i = 0U; i < sizeof(wallet_id); ++i) wallet_id[i] = (uint8_t)i;
    bc2_device_service_set_wallet_id(&service, wallet_id);
    prepare_request(&usb, BC2_USB_CMD_GET_WALLET_ID, 933U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U + BC2_DEVICE_WALLET_ID_SIZE);
    assert(response.payload[0] == 1U);
    assert(memcmp(response.payload + 1U, wallet_id, sizeof(wallet_id)) == 0);

    machine.state = BC2_DEVICE_LOCKED;
    prepare_request(&usb, BC2_USB_CMD_GET_WALLET_ID, 934U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 0U);
    bc2_device_service_clear_wallet_id(&service);
    machine.state = BC2_DEVICE_DASHBOARD;

    /* v0.34 receive flow: desktop requests an address, but the address itself
     * is only returned after the hardware completes physical approval. */
    prepare_request(&usb, BC2_USB_CMD_BEGIN_RECEIVE_ADDRESS, 94U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_receive_request(&service));
    assert(!bc2_device_service_take_receive_request(&service));

    prepare_request(&usb, BC2_USB_CMD_GET_RECEIVE_RESULT, 95U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U &&
           response.payload[0] == BC2_DEVICE_REVIEW_PENDING);

    const char approved_receive[] = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4";
    bc2_device_service_complete_receive(&service, 1, approved_receive);
    prepare_request(&usb, BC2_USB_CMD_GET_RECEIVE_RESULT, 96U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload[0] == BC2_DEVICE_REVIEW_APPROVED);
    assert(response.payload[1] == strlen(approved_receive));
    assert(response.payload_length == 2U + strlen(approved_receive));
    assert(memcmp(response.payload + 2U, approved_receive,
                  strlen(approved_receive)) == 0);

    const uint8_t address[] = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4";
    prepare_request(&usb, BC2_USB_CMD_REVIEW_RECEIVE_ADDRESS, 10U,
                    address, (uint16_t)(sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    char queued[BC2_DEVICE_RECEIVE_ADDRESS_MAX];
    assert(bc2_device_service_take_receive_address(&service, queued, sizeof(queued)));
    assert(strcmp(queued, (const char *)address) == 0);
    assert(!bc2_device_service_take_receive_address(&service, queued, sizeof(queued)));

    const uint8_t invalid_address[] = "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5";
    prepare_request(&usb, BC2_USB_CMD_REVIEW_RECEIVE_ADDRESS, 11U,
                    invalid_address, (uint16_t)(sizeof(invalid_address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 0U);

    machine.state = BC2_DEVICE_LOCKED;
    prepare_request(&usb, BC2_USB_CMD_REVIEW_RECEIVE_ADDRESS, 12U,
                    address, (uint16_t)(sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 2U);

    machine.state = BC2_DEVICE_DASHBOARD;
    uint8_t transaction[128] = {1U, 1U, 1U};
    write_u64_le(transaction + 3U, 50000U);
    write_u64_le(transaction + 11U, 12000U);
    write_u64_le(transaction + 19U, 500U);
    transaction[27] = (uint8_t)(sizeof(address) - 1U);
    memcpy(transaction + 28U, address, sizeof(address) - 1U);
    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 13U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    bc2_device_transaction_review_t queued_transaction;
    assert(bc2_device_service_take_transaction(&service, &queued_transaction));
    assert(queued_transaction.recipient_amount == 50000U);
    assert(queued_transaction.change_amount == 12000U);
    assert(queued_transaction.fee_amount == 500U);
    assert(strcmp(queued_transaction.recipient_address, (const char *)address) == 0);
    assert(!bc2_device_service_take_transaction(&service, &queued_transaction));

    /* Taking the queued data starts the physical review; it must not make the
     * service appear idle while PIN entry or the review screen is active. */
    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 131U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 3U);

    prepare_request(&usb, BC2_USB_CMD_GET_TRANSACTION_RESULT, 14U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U &&
           response.payload[0] == BC2_DEVICE_REVIEW_PENDING);

    bc2_device_service_complete_transaction(&service, 1);
    prepare_request(&usb, BC2_USB_CMD_GET_TRANSACTION_RESULT, 15U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U &&
           response.payload[0] == BC2_DEVICE_REVIEW_APPROVED);

    /* A terminal decision must survive repeated reads.  Building or sending a
     * response is not an acknowledgement that the desktop received it. */
    prepare_request(&usb, BC2_USB_CMD_GET_TRANSACTION_RESULT, 16U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U &&
           response.payload[0] == BC2_DEVICE_REVIEW_APPROVED);

    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 17U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_transaction(&service, &queued_transaction));
    bc2_device_service_complete_transaction(&service, 0);
    prepare_request(&usb, BC2_USB_CMD_GET_TRANSACTION_RESULT, 18U, NULL, 0U);
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U &&
           response.payload[0] == BC2_DEVICE_REVIEW_REJECTED);

    /* A completed result that was not polled must not block a new review. */
    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 181U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_transaction(&service, &queued_transaction));
    bc2_device_service_complete_transaction(&service, 0);

    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 182U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 1U);
    assert(bc2_device_service_take_transaction(&service, &queued_transaction));
    bc2_device_service_complete_transaction(&service, 0);

    transaction[1] = 0U;
    prepare_request(&usb, BC2_USB_CMD_REVIEW_TRANSACTION, 19U, transaction,
                    (uint16_t)(28U + sizeof(address) - 1U));
    process_until_response(&service, &hal, &machine, &identity, &usb);
    assert(bc2_usb_parse(usb.response, usb.response_size, &response) == BC2_USB_PARSE_OK);
    assert(response.payload_length == 1U && response.payload[0] == 0U);
    return 0;
}
