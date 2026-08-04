#ifndef BC2_DEVICE_SERVICE_H
#define BC2_DEVICE_SERVICE_H

#include "bc2_device_state.h"
#include "bc2_hal.h"
#include "bc2_usb_stream.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_DEVICE_FIRMWARE_VERSION "0.29.7"
#define BC2_DEVICE_INFO_MAX 192U
#define BC2_DEVICE_RECEIVE_ADDRESS_MAX 96U
#define BC2_DEVICE_TRANSACTION_ADDRESS_MAX 96U

typedef struct {
    uint64_t recipient_amount;
    uint64_t change_amount;
    uint64_t fee_amount;
    char recipient_address[BC2_DEVICE_TRANSACTION_ADDRESS_MAX];
} bc2_device_transaction_review_t;

typedef enum {
    BC2_DEVICE_REVIEW_PENDING = 0,
    BC2_DEVICE_REVIEW_APPROVED = 1,
    BC2_DEVICE_REVIEW_REJECTED = 2,
    BC2_DEVICE_REVIEW_NONE = 3
} bc2_device_review_result_t;

typedef enum {
    BC2_DEVICE_CAP_USB = 1U << 0,
    BC2_DEVICE_CAP_STORAGE = 1U << 1,
    BC2_DEVICE_CAP_RANDOM = 1U << 2,
    BC2_DEVICE_CAP_DISPLAY = 1U << 3,
    BC2_DEVICE_CAP_BUTTONS = 1U << 4
} bc2_device_capability_t;

typedef struct {
    const char *board_name;
    uint16_t display_width;
    uint16_t display_height;
    uint8_t board_revision;
    uint8_t capabilities;
} bc2_device_identity_t;

typedef struct {
    bc2_usb_stream_t usb_stream;
    char pending_receive_address[BC2_DEVICE_RECEIVE_ADDRESS_MAX];
    int receive_address_pending;
    bc2_device_transaction_review_t pending_transaction;
    int transaction_pending;
    int transaction_review_active;
    bc2_device_review_result_t transaction_result;
} bc2_device_service_t;

void bc2_device_service_init(bc2_device_service_t *service);

bc2_hal_result_t bc2_device_service_process_usb(bc2_device_service_t *service,
                                                const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity);
int bc2_device_service_take_receive_address(bc2_device_service_t *service,
                                            char *output,
                                            size_t output_capacity);
int bc2_device_service_take_transaction(bc2_device_service_t *service,
                                        bc2_device_transaction_review_t *output);
void bc2_device_service_complete_transaction(bc2_device_service_t *service,
                                             int approved);

#ifdef __cplusplus
}
#endif

#endif
