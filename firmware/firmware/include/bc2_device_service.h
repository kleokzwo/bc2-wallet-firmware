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

#define BC2_DEVICE_FIRMWARE_VERSION "0.49.5"
#define BC2_DEVICE_INFO_MAX 192U
#define BC2_DEVICE_RECEIVE_ADDRESS_MAX 96U
#define BC2_DEVICE_TRANSACTION_ADDRESS_MAX 96U
#define BC2_DEVICE_RECOVERY_MNEMONIC_MAX 256U
#define BC2_DEVICE_WALLET_ID_SIZE 16U

typedef struct {
    uint64_t recipient_amount;
    uint64_t change_amount;
    uint64_t fee_amount;
    char recipient_address[BC2_DEVICE_TRANSACTION_ADDRESS_MAX];
} bc2_device_transaction_review_t;
typedef struct {
    uint8_t prev_txid_le[32];
    uint32_t prev_output_index;
    uint64_t input_amount;
    uint32_t input_index;
    uint8_t input_count;
    uint8_t input_position;
    uint8_t hash_prevouts[32];
    uint8_t hash_sequence[32];
    char input_address[BC2_DEVICE_TRANSACTION_ADDRESS_MAX];
    char change_address[BC2_DEVICE_TRANSACTION_ADDRESS_MAX];
} bc2_device_sign_request_t;

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
    int receive_request_pending;
    int receive_review_active;
    bc2_device_review_result_t receive_result;
    char approved_receive_address[BC2_DEVICE_RECEIVE_ADDRESS_MAX];
    bc2_device_transaction_review_t pending_transaction;
    int transaction_pending;
    int transaction_review_active;
    bc2_device_review_result_t transaction_result;
    bc2_device_transaction_review_t reviewed_transaction;
    bc2_device_sign_request_t pending_sign;
    int sign_pending;
    uint8_t sign_status;
    uint8_t sign_public_key[33];
    uint8_t sign_signature[80];
    size_t sign_signature_length;
    uint8_t sign_session_count;
    uint8_t sign_session_next_position;
    uint64_t sign_session_total_amount;
    uint8_t sign_session_hash_prevouts[32];
    uint8_t sign_session_hash_sequence[32];
    char sign_session_change_address[BC2_DEVICE_TRANSACTION_ADDRESS_MAX];
    int create_wallet_pending;
    int recovery_pending;
    int recovery_input_enabled;
    int recovery_mnemonic_pending;
    size_t recovery_mnemonic_length;
    char recovery_mnemonic[BC2_DEVICE_RECOVERY_MNEMONIC_MAX];
    int unlock_pending;
    int lock_pending;
    uint8_t wallet_id[BC2_DEVICE_WALLET_ID_SIZE];
    int wallet_id_available;
} bc2_device_service_t;

void bc2_device_service_init(bc2_device_service_t *service);

bc2_hal_result_t bc2_device_service_process_usb(bc2_device_service_t *service,
                                                const bc2_hal_t *hal,
                                                const bc2_device_machine *machine,
                                                const bc2_device_identity_t *identity);
/* Legacy desktop-supplied address review. Kept only for protocol compatibility. */
int bc2_device_service_take_receive_address(bc2_device_service_t *service,
                                            char *output,
                                            size_t output_capacity);

/* v0.34+: hardware-owned receive flow. */
int bc2_device_service_take_receive_request(bc2_device_service_t *service);
void bc2_device_service_complete_receive(bc2_device_service_t *service,
                                         int approved,
                                         const char *address);
int bc2_device_service_take_transaction(bc2_device_service_t *service,
                                        bc2_device_transaction_review_t *output);
void bc2_device_service_complete_transaction(bc2_device_service_t *service,
                                             int approved);
int bc2_device_service_take_sign_request(bc2_device_service_t *service,bc2_device_sign_request_t *output);
void bc2_device_service_complete_sign(
    bc2_device_service_t *service,
    uint8_t status,
    const uint8_t public_key[33],
    const uint8_t *signature,
    size_t signature_length);
int bc2_device_service_take_create_wallet(bc2_device_service_t *service);
int bc2_device_service_take_recovery(bc2_device_service_t *service);
int bc2_device_service_take_recovery_mnemonic(bc2_device_service_t *service,
                                              char *output, size_t output_capacity);
void bc2_device_service_cancel_recovery(bc2_device_service_t *service);
int bc2_device_service_take_unlock(bc2_device_service_t *service);
int bc2_device_service_take_lock(bc2_device_service_t *service);
void bc2_device_service_set_wallet_id(bc2_device_service_t *service,
                                      const uint8_t wallet_id[BC2_DEVICE_WALLET_ID_SIZE]);
void bc2_device_service_clear_wallet_id(bc2_device_service_t *service);

#ifdef __cplusplus
}
#endif

#endif
