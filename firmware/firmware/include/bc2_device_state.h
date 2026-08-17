#ifndef BC2_DEVICE_STATE_H
#define BC2_DEVICE_STATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BC2_DEVICE_BOOT = 0,
    BC2_DEVICE_SETUP_REQUIRED,
    BC2_DEVICE_LOCKED,
    BC2_DEVICE_UNLOCKING,
    BC2_DEVICE_COOLDOWN,
    BC2_DEVICE_DASHBOARD,
    BC2_DEVICE_RECEIVE_REVIEW,
    BC2_DEVICE_TRANSACTION_REVIEW,
    BC2_DEVICE_SETTINGS,
    BC2_DEVICE_ERROR
} bc2_device_state;

typedef enum {
    BC2_DEVICE_EVENT_BOOT_COMPLETE = 0,
    BC2_DEVICE_EVENT_SETUP_COMPLETE,
    BC2_DEVICE_EVENT_LOCK,
    BC2_DEVICE_EVENT_BEGIN_UNLOCK,
    BC2_DEVICE_EVENT_UNLOCK_SUCCESS,
    BC2_DEVICE_EVENT_UNLOCK_FAILURE,
    BC2_DEVICE_EVENT_COOLDOWN_EXPIRED,
    BC2_DEVICE_EVENT_OPEN_RECEIVE,
    BC2_DEVICE_EVENT_OPEN_TRANSACTION,
    BC2_DEVICE_EVENT_OPEN_SETTINGS,
    BC2_DEVICE_EVENT_CONFIRM,
    BC2_DEVICE_EVENT_CANCEL,
    BC2_DEVICE_EVENT_ACTIVITY,
    BC2_DEVICE_EVENT_SESSION_TIMEOUT,
    BC2_DEVICE_EVENT_FATAL_ERROR,
    BC2_DEVICE_EVENT_RECOVER
} bc2_device_event;

typedef enum {
    BC2_DEVICE_ACTION_NONE = 0,
    BC2_DEVICE_ACTION_UNLOCKED,
    BC2_DEVICE_ACTION_LOCKED,
    BC2_DEVICE_ACTION_RECEIVE_CONFIRMED,
    BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED,
    BC2_DEVICE_ACTION_SETTINGS_CONFIRMED,
    BC2_DEVICE_ACTION_CANCELLED,
    BC2_DEVICE_ACTION_COOLDOWN_STARTED,
    BC2_DEVICE_ACTION_FATAL_ERROR
} bc2_device_action;

typedef struct {
    bc2_device_state state;
    bc2_device_state state_before_error;
    bc2_device_action last_action;
    uint32_t failed_unlock_attempts;
    uint32_t max_unlock_attempts;
    uint32_t cooldown_level;
    uint64_t cooldown_until_ms;
    uint64_t last_activity_ms;
    uint64_t session_timeout_ms;
    int wallet_is_initialized;
} bc2_device_machine;

void bc2_device_machine_init(bc2_device_machine *machine, int wallet_is_initialized, uint64_t now_ms);
int bc2_device_machine_dispatch(bc2_device_machine *machine, bc2_device_event event, uint64_t now_ms);
int bc2_device_machine_tick(bc2_device_machine *machine, uint64_t now_ms);
uint64_t bc2_device_machine_cooldown_remaining(const bc2_device_machine *machine, uint64_t now_ms);
int bc2_device_machine_is_unlocked(const bc2_device_machine *machine);
const char *bc2_device_state_name(bc2_device_state state);
const char *bc2_device_action_name(bc2_device_action action);

#ifdef __cplusplus
}
#endif

#endif
