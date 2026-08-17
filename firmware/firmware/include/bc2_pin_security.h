#ifndef BC2_PIN_SECURITY_H
#define BC2_PIN_SECURITY_H

#include "bc2_hal.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_SECURITY_PIN_LENGTH 4U
#define BC2_SECURITY_PIN_MAX_FAILURES 10U

typedef enum {
    BC2_PIN_SECURITY_OK = 0,
    BC2_PIN_SECURITY_NOT_CONFIGURED,
    BC2_PIN_SECURITY_INVALID,
    BC2_PIN_SECURITY_DELAYED,
    BC2_PIN_SECURITY_ERROR
} bc2_pin_security_result_t;

typedef enum {
    BC2_AUTH_UNLOCK = 0,
    BC2_AUTH_RECEIVE_ADDRESS,
    BC2_AUTH_TRANSACTION,
    BC2_AUTH_NEW_WALLET
} bc2_authorization_action_t;

typedef enum {
    BC2_AUTH_DEVICE_PIN = 0,
    BC2_AUTH_ROOT_PIN
} bc2_authorization_pin_t;

typedef struct {
    bool configured;
    uint8_t failures;
    uint64_t blocked_until_ms;
} bc2_pin_security_t;

bc2_pin_security_result_t bc2_pin_security_load(bc2_pin_security_t *security,
                                                const bc2_hal_t *hal,
                                                uint64_t now_ms);
bc2_pin_security_result_t bc2_pin_security_create(bc2_pin_security_t *security,
                                                  const bc2_hal_t *hal,
                                                  const char *pin);
bc2_pin_security_result_t bc2_pin_security_verify(bc2_pin_security_t *security,
                                                  const bc2_hal_t *hal,
                                                  const char *pin,
                                                  uint64_t now_ms);
bc2_pin_security_result_t bc2_pin_security_reset(bc2_pin_security_t *security, const bc2_hal_t *hal);
uint64_t bc2_pin_security_remaining_delay(const bc2_pin_security_t *security,
                                          uint64_t now_ms);
bc2_authorization_pin_t bc2_authorization_required_pin(bc2_authorization_action_t action);

#ifdef __cplusplus
}
#endif

#endif
