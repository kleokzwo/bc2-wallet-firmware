#include "bc2_device_service.h"

#include "bc2_usb_protocol.h"
#include "bc2_receive_request.h"

#include <stdio.h>
#include <string.h>

#define BC2_USB_RESPONSE_FLAG 0x80U
#define BC2_TRANSACTION_PAYLOAD_FIXED_SIZE 28U

static uint64_t read_u64_le(const uint8_t *bytes) {
    uint64_t value = 0U;
    for (unsigned int index = 0U; index < 8U; ++index)
        value |= ((uint64_t)bytes[index]) << (index * 8U);
    return value;
}

static int queue_transaction(bc2_device_service_t *service,
                             const uint8_t *payload,
                             size_t payload_length) {
    size_t address_length;
    uint64_t total;
    if (service == NULL || payload == NULL || payload_length < BC2_TRANSACTION_PAYLOAD_FIXED_SIZE ||
        service->transaction_review_active)
        return 0;
    if (payload[0] != 1U || payload[1] != 1U || payload[2] != 1U) return 0;
    address_length = payload[27];
    if (address_length == 0U || address_length >= BC2_DEVICE_TRANSACTION_ADDRESS_MAX ||
        payload_length != BC2_TRANSACTION_PAYLOAD_FIXED_SIZE + address_length) return 0;
    service->pending_transaction.recipient_amount = read_u64_le(payload + 3U);
    service->pending_transaction.change_amount = read_u64_le(payload + 11U);
    service->pending_transaction.fee_amount = read_u64_le(payload + 19U);
    if (service->pending_transaction.recipient_amount == 0U) return 0;
    total = service->pending_transaction.recipient_amount + service->pending_transaction.change_amount;
    if (total < service->pending_transaction.recipient_amount ||
        total + service->pending_transaction.fee_amount < total) return 0;
    memcpy(service->pending_transaction.recipient_address, payload + 28U, address_length);
    service->pending_transaction.recipient_address[address_length] = '\0';
    if (!bc2_receive_address_is_valid(service->pending_transaction.recipient_address)) {
        memset(&service->pending_transaction, 0, sizeof(service->pending_transaction));
        return 0;
    }
    service->transaction_pending = 1;
    service->transaction_review_active = 1;
    service->transaction_result = BC2_DEVICE_REVIEW_PENDING;
    return 1;
}

static size_t build_info_payload(const bc2_device_identity_t *identity,
                                 char *output,
                                 size_t output_capacity) {
    if (identity == NULL || output == NULL || output_capacity == 0U) return 0U;

    const int written = snprintf(output,
                                 output_capacity,
                                 "BC2 Cold Wallet %s\n%s\n%ux%u\nrevision=%u",
                                 BC2_DEVICE_FIRMWARE_VERSION,
                                 identity->board_name != NULL ? identity->board_name : "Unknown board",
                                 (unsigned int)identity->display_width,
                                 (unsigned int)identity->display_height,
                                 (unsigned int)identity->board_revision);
    if (written < 0 || (size_t)written >= output_capacity) return 0U;
    return (size_t)written;
}

static size_t build_response(const bc2_usb_message_t *request,
                             const bc2_device_machine *machine,
                             const bc2_device_identity_t *identity,
                             bc2_device_service_t *service,
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
        case BC2_USB_CMD_GET_CAPABILITIES:
            if (identity == NULL) return 0U;
            payload[0] = identity->capabilities;
            payload[1] = identity->board_revision;
            payload_size = 2U;
            break;
        case BC2_USB_CMD_GET_WALLET_STATUS:
            if (machine == NULL) return 0U;
            payload[0] = machine->wallet_is_initialized ? 2U : 0U;
            payload_size = 1U;
            break;
        case BC2_USB_CMD_BEGIN_CREATE_WALLET:
            payload[0] = 0U;
            payload_size = 1U;
            if (service == NULL || machine == NULL ||
                machine->state != BC2_DEVICE_SETUP_REQUIRED ||
                service->create_wallet_pending)
                break;
            service->create_wallet_pending = 1;
            payload[0] = 1U;
            break;
case BC2_USB_CMD_BEGIN_RECOVERY:
    payload[0] = 0U;
    payload_size = 1U;
    if (service == NULL || machine == NULL ||
        (machine->state != BC2_DEVICE_SETUP_REQUIRED &&
         machine->state != BC2_DEVICE_LOCKED) ||
        service->recovery_pending)
        break;
    service->recovery_pending = 1;
    service->recovery_input_enabled = 1;
    payload[0] = 1U;
    break;
case BC2_USB_CMD_SUBMIT_RECOVERY_MNEMONIC:
    payload[0] = 0U;
    payload_size = 1U;
    if (service == NULL || machine == NULL ||
        !service->recovery_input_enabled || service->recovery_mnemonic_pending ||
        (machine->state != BC2_DEVICE_SETUP_REQUIRED && machine->state != BC2_DEVICE_LOCKED) ||
        request->payload_length == 0U || request->payload_length >= BC2_DEVICE_RECOVERY_MNEMONIC_MAX)
        break;
    memcpy(service->recovery_mnemonic, request->payload, request->payload_length);
    service->recovery_mnemonic[request->payload_length] = '\0';
    service->recovery_mnemonic_length = request->payload_length;
    service->recovery_mnemonic_pending = 1;
    service->recovery_input_enabled = 0;
    payload[0] = 1U;
    break;
case BC2_USB_CMD_BEGIN_UNLOCK:
    payload[0] = 0U;
    payload_size = 1U;
    if (service == NULL || machine == NULL ||
        machine->state != BC2_DEVICE_LOCKED || service->unlock_pending)
        break;
    service->unlock_pending = 1;
    payload[0] = 1U;
    break;
case BC2_USB_CMD_LOCK_WALLET:
    payload[0] = 0U;
    payload_size = 1U;
    if (service == NULL || machine == NULL ||
        !bc2_device_machine_is_unlocked(machine) || service->lock_pending)
        break;
    service->lock_pending = 1;
    payload[0] = 1U;
    break;
case BC2_USB_CMD_BEGIN_RECEIVE_ADDRESS:
    payload[0] = 0U;
    payload_size = 1U;
    if (service == NULL || machine == NULL ||
        machine->state != BC2_DEVICE_DASHBOARD) {
        payload[0] = 2U;
        break;
    }
    if (service->receive_review_active ||
        service->transaction_review_active ||
        service->create_wallet_pending) {
        payload[0] = 3U;
        break;
    }
    service->receive_request_pending = 1;
    service->receive_review_active = 1;
    service->receive_result = BC2_DEVICE_REVIEW_PENDING;
    memset(service->approved_receive_address, 0,
           sizeof(service->approved_receive_address));
    payload[0] = 1U;
    break;

case BC2_USB_CMD_GET_RECEIVE_RESULT: {
    size_t address_length = 0U;
    if (service == NULL || request->payload_length != 0U) return 0U;
    payload[0] = (uint8_t)service->receive_result;
    payload_size = 1U;
    if (service->receive_result == BC2_DEVICE_REVIEW_APPROVED) {
        address_length = strlen(service->approved_receive_address);
        if (address_length == 0U ||
            address_length >= BC2_DEVICE_RECEIVE_ADDRESS_MAX ||
            address_length > 255U ||
            address_length + 2U > sizeof(payload))
            return 0U;
        payload[1] = (uint8_t)address_length;
        memcpy(payload + 2U, service->approved_receive_address,
               address_length);
        payload_size = address_length + 2U;
    }
    break;
}

        case BC2_USB_CMD_REVIEW_RECEIVE_ADDRESS: {
            payload[0] = 0U;
            payload_size = 1U;
            if (service == NULL || machine == NULL || machine->state != BC2_DEVICE_DASHBOARD) {
                payload[0] = 2U;
                break;
            }
            if (request->payload_length == 0U ||
                request->payload_length >= sizeof(service->pending_receive_address)) break;
            memcpy(service->pending_receive_address, request->payload, request->payload_length);
            service->pending_receive_address[request->payload_length] = '\0';
            if (!bc2_receive_address_is_valid(service->pending_receive_address)) {
                memset(service->pending_receive_address, 0,
                       sizeof(service->pending_receive_address));
                break;
            }
            service->receive_address_pending = 1;
            payload[0] = 1U;
            break;
        }
        case BC2_USB_CMD_REVIEW_TRANSACTION:
            payload[0] = 0U;
            payload_size = 1U;
            if (service == NULL || machine == NULL || machine->state != BC2_DEVICE_DASHBOARD) {
                payload[0] = 2U;
                break;
            }
            if (service->transaction_review_active) {
                payload[0] = 3U;
                break;
            }
            if (queue_transaction(service, request->payload, request->payload_length))
                payload[0] = 1U;
            break;
        case BC2_USB_CMD_GET_TRANSACTION_RESULT:
            if (service == NULL || request->payload_length != 0U) return 0U;
            payload[0] = (uint8_t)service->transaction_result;
            payload_size = 1U;
            /* The transport may fail after this response has been built.  Keep
             * a terminal result idempotent so a retry cannot lose the user's
             * physical decision.  queue_transaction() replaces it with
             * PENDING only after a new request has been validated. */
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

void bc2_device_service_init(bc2_device_service_t *service) {
    if (service != NULL) {
        memset(service, 0, sizeof(*service));
        service->transaction_result = BC2_DEVICE_REVIEW_NONE;
        service->receive_result = BC2_DEVICE_REVIEW_NONE;
        bc2_usb_stream_init(&service->usb_stream);
    }
}

bc2_hal_result_t bc2_device_service_process_usb(bc2_device_service_t *service,
                                                const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity) {
    uint8_t receive_buffer[BC2_HAL_USB_MAX_MESSAGE];
    uint8_t request_buffer[BC2_USB_FRAME_MAX];
    uint8_t response_buffer[BC2_HAL_USB_MAX_MESSAGE];
    size_t receive_size = 0U;
    size_t request_size = 0U;
    bc2_usb_message_t request;

    if (service == NULL) return BC2_HAL_ERROR_ARGUMENT;

    const bc2_hal_result_t receive_result = bc2_hal_usb_receive(hal,
                                                               receive_buffer,
                                                               sizeof(receive_buffer),
                                                               &receive_size);
    if (receive_result != BC2_HAL_OK && receive_result != BC2_HAL_ERROR_NOT_FOUND)
        return receive_result;
    if (receive_size > 0U &&
        bc2_usb_stream_push(&service->usb_stream, receive_buffer, receive_size) == 0)
        return BC2_HAL_ERROR_LIMIT;

    const int next_result = bc2_usb_stream_next(&service->usb_stream,
                                                request_buffer,
                                                sizeof(request_buffer),
                                                &request_size);
    if (next_result == 0) return BC2_HAL_ERROR_NOT_FOUND;
    if (next_result < 0) return BC2_HAL_ERROR_IO;
    if (bc2_usb_parse(request_buffer, request_size, &request) != BC2_USB_PARSE_OK)
        return BC2_HAL_ERROR_IO;

    const size_t response_size = build_response(&request,
                                                machine,
                                                identity,
                                                service,
                                                response_buffer,
                                                sizeof(response_buffer));
    if (response_size == 0U) return BC2_HAL_ERROR_UNAVAILABLE;
    return bc2_hal_usb_send(hal, response_buffer, response_size);
}

int bc2_device_service_take_receive_address(bc2_device_service_t *service,
                                            char *output,
                                            size_t output_capacity) {
    size_t length;
    if (service == NULL || output == NULL || output_capacity == 0U ||
        !service->receive_address_pending) return 0;
    length = strlen(service->pending_receive_address);
    if (length + 1U > output_capacity) return 0;
    memcpy(output, service->pending_receive_address, length + 1U);
    memset(service->pending_receive_address, 0, sizeof(service->pending_receive_address));
    service->receive_address_pending = 0;
    return 1;
}


int bc2_device_service_take_receive_request(bc2_device_service_t *service) {
    if (service == NULL || !service->receive_request_pending) return 0;
    service->receive_request_pending = 0;
    return 1;
}

void bc2_device_service_complete_receive(bc2_device_service_t *service,
                                         int approved,
                                         const char *address) {
    size_t length = 0U;
    if (service == NULL || !service->receive_review_active ||
        service->receive_result != BC2_DEVICE_REVIEW_PENDING)
        return;

    memset(service->approved_receive_address, 0,
           sizeof(service->approved_receive_address));

    if (approved && address != NULL) {
        length = strlen(address);
        if (length > 0U && length < sizeof(service->approved_receive_address)) {
            memcpy(service->approved_receive_address, address, length + 1U);
            service->receive_result = BC2_DEVICE_REVIEW_APPROVED;
        } else {
            service->receive_result = BC2_DEVICE_REVIEW_REJECTED;
        }
    } else {
        service->receive_result = BC2_DEVICE_REVIEW_REJECTED;
    }

    service->receive_request_pending = 0;
    service->receive_review_active = 0;
}

int bc2_device_service_take_transaction(bc2_device_service_t *service,
                                        bc2_device_transaction_review_t *output) {
    if (service == NULL || output == NULL || !service->transaction_pending) return 0;
    *output = service->pending_transaction;
    memset(&service->pending_transaction, 0, sizeof(service->pending_transaction));
    service->transaction_pending = 0;
    return 1;
}

void bc2_device_service_complete_transaction(bc2_device_service_t *service,
                                             int approved) {
    if (service == NULL || !service->transaction_review_active ||
        service->transaction_result != BC2_DEVICE_REVIEW_PENDING) return;
    service->transaction_pending = 0;
    service->transaction_review_active = 0;
    service->transaction_result = approved
        ? BC2_DEVICE_REVIEW_APPROVED
        : BC2_DEVICE_REVIEW_REJECTED;
}

int bc2_device_service_take_create_wallet(bc2_device_service_t *service) {
    if (service == NULL || !service->create_wallet_pending) return 0;
    service->create_wallet_pending = 0;
    return 1;
}

int bc2_device_service_take_recovery(bc2_device_service_t *service) {
    if (service == NULL || !service->recovery_pending) return 0;
    service->recovery_pending = 0;
    return 1;
}

int bc2_device_service_take_recovery_mnemonic(bc2_device_service_t *service,
                                              char *output, size_t output_capacity) {
    size_t length;
    if (service == NULL || output == NULL || output_capacity == 0U ||
        !service->recovery_mnemonic_pending) return 0;
    length = service->recovery_mnemonic_length;
    if (length + 1U > output_capacity) return 0;
    memcpy(output, service->recovery_mnemonic, length + 1U);
    memset(service->recovery_mnemonic, 0, sizeof(service->recovery_mnemonic));
    service->recovery_mnemonic_length = 0U;
    service->recovery_mnemonic_pending = 0;
    return 1;
}

void bc2_device_service_cancel_recovery(bc2_device_service_t *service) {
    if (service == NULL) return;
    memset(service->recovery_mnemonic, 0, sizeof(service->recovery_mnemonic));
    service->recovery_mnemonic_length = 0U;
    service->recovery_mnemonic_pending = 0;
    service->recovery_pending = 0;
    service->recovery_input_enabled = 0;
}

int bc2_device_service_take_unlock(bc2_device_service_t *service) {
    if (service == NULL || !service->unlock_pending) return 0;
    service->unlock_pending = 0;
    return 1;
}


int bc2_device_service_take_lock(bc2_device_service_t *service) {
    if (service == NULL || !service->lock_pending) return 0;
    service->lock_pending = 0;
    return 1;
}
