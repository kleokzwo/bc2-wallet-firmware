#include "bc2_device_state.h"

#include <limits.h>

#define BC2_DEFAULT_MAX_UNLOCK_ATTEMPTS 5U
#define BC2_DEFAULT_SESSION_TIMEOUT_MS (5ULL * 60ULL * 1000ULL)
#define BC2_BASE_COOLDOWN_MS 30000ULL
#define BC2_MAX_COOLDOWN_MS (24ULL * 60ULL * 60ULL * 1000ULL)

static int state_is_unlocked(bc2_device_state state) {
    return state == BC2_DEVICE_DASHBOARD || state == BC2_DEVICE_RECEIVE_REVIEW ||
           state == BC2_DEVICE_TRANSACTION_REVIEW || state == BC2_DEVICE_SETTINGS;
}

static uint64_t cooldown_duration(uint32_t level) {
    uint64_t duration = BC2_BASE_COOLDOWN_MS;
    uint32_t i;
    for (i = 1U; i < level && duration < BC2_MAX_COOLDOWN_MS; ++i) {
        if (duration > BC2_MAX_COOLDOWN_MS / 2ULL) return BC2_MAX_COOLDOWN_MS;
        duration *= 2ULL;
    }
    return duration > BC2_MAX_COOLDOWN_MS ? BC2_MAX_COOLDOWN_MS : duration;
}

static void lock_machine(bc2_device_machine *machine, uint64_t now_ms) {
    machine->state = BC2_DEVICE_LOCKED;
    machine->last_action = BC2_DEVICE_ACTION_LOCKED;
    machine->last_activity_ms = now_ms;
}

void bc2_device_machine_init(bc2_device_machine *machine, int wallet_is_initialized, uint64_t now_ms) {
    if (machine == 0) return;
    machine->state = BC2_DEVICE_BOOT;
    machine->state_before_error = BC2_DEVICE_BOOT;
    machine->last_action = BC2_DEVICE_ACTION_NONE;
    machine->failed_unlock_attempts = 0U;
    machine->max_unlock_attempts = BC2_DEFAULT_MAX_UNLOCK_ATTEMPTS;
    machine->cooldown_level = 0U;
    machine->cooldown_until_ms = 0U;
    machine->last_activity_ms = now_ms;
    machine->session_timeout_ms = BC2_DEFAULT_SESSION_TIMEOUT_MS;
    machine->wallet_is_initialized = wallet_is_initialized ? 1 : 0;
}

int bc2_device_machine_dispatch(bc2_device_machine *machine, bc2_device_event event, uint64_t now_ms) {
    bc2_device_state previous;
    if (machine == 0) return 0;
    machine->last_action = BC2_DEVICE_ACTION_NONE;

    if (event == BC2_DEVICE_EVENT_FATAL_ERROR) {
        machine->state_before_error = machine->state;
        machine->state = BC2_DEVICE_ERROR;
        machine->last_action = BC2_DEVICE_ACTION_FATAL_ERROR;
        return 1;
    }

    if (event == BC2_DEVICE_EVENT_LOCK && state_is_unlocked(machine->state)) {
        lock_machine(machine, now_ms);
        return 1;
    }

    if (event == BC2_DEVICE_EVENT_SESSION_TIMEOUT && state_is_unlocked(machine->state)) {
        lock_machine(machine, now_ms);
        return 1;
    }

    if (event == BC2_DEVICE_EVENT_ACTIVITY && state_is_unlocked(machine->state)) {
        machine->last_activity_ms = now_ms;
        return 1;
    }

    previous = machine->state;
    switch (machine->state) {
        case BC2_DEVICE_BOOT:
            if (event == BC2_DEVICE_EVENT_BOOT_COMPLETE) {
                machine->state = machine->wallet_is_initialized ? BC2_DEVICE_LOCKED : BC2_DEVICE_SETUP_REQUIRED;
            }
            break;
        case BC2_DEVICE_SETUP_REQUIRED:
            if (event == BC2_DEVICE_EVENT_SETUP_COMPLETE) {
                machine->wallet_is_initialized = 1;
                lock_machine(machine, now_ms);
            }
            break;
        case BC2_DEVICE_LOCKED:
            if (event == BC2_DEVICE_EVENT_BEGIN_UNLOCK) machine->state = BC2_DEVICE_UNLOCKING;
            break;
        case BC2_DEVICE_UNLOCKING:
            if (event == BC2_DEVICE_EVENT_UNLOCK_SUCCESS) {
                machine->failed_unlock_attempts = 0U;
                machine->cooldown_level = 0U;
                machine->cooldown_until_ms = 0U;
                machine->last_activity_ms = now_ms;
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_UNLOCKED;
            } else if (event == BC2_DEVICE_EVENT_UNLOCK_FAILURE) {
                if (machine->failed_unlock_attempts < UINT_MAX) ++machine->failed_unlock_attempts;
                if (machine->failed_unlock_attempts >= machine->max_unlock_attempts) {
                    if (machine->cooldown_level < UINT_MAX) ++machine->cooldown_level;
                    machine->cooldown_until_ms = now_ms + cooldown_duration(machine->cooldown_level);
                    machine->state = BC2_DEVICE_COOLDOWN;
                    machine->last_action = BC2_DEVICE_ACTION_COOLDOWN_STARTED;
                } else {
                    lock_machine(machine, now_ms);
                }
            } else if (event == BC2_DEVICE_EVENT_CANCEL) {
                lock_machine(machine, now_ms);
                machine->last_action = BC2_DEVICE_ACTION_CANCELLED;
            }
            break;
        case BC2_DEVICE_COOLDOWN:
            if (event == BC2_DEVICE_EVENT_COOLDOWN_EXPIRED && now_ms >= machine->cooldown_until_ms) {
                machine->failed_unlock_attempts = 0U;
                lock_machine(machine, now_ms);
            }
            break;
        case BC2_DEVICE_DASHBOARD:
            if (event == BC2_DEVICE_EVENT_OPEN_RECEIVE) machine->state = BC2_DEVICE_RECEIVE_REVIEW;
            else if (event == BC2_DEVICE_EVENT_OPEN_TRANSACTION) machine->state = BC2_DEVICE_TRANSACTION_REVIEW;
            else if (event == BC2_DEVICE_EVENT_OPEN_SETTINGS) machine->state = BC2_DEVICE_SETTINGS;
            break;
        case BC2_DEVICE_RECEIVE_REVIEW:
            if (event == BC2_DEVICE_EVENT_CONFIRM) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_RECEIVE_CONFIRMED;
            } else if (event == BC2_DEVICE_EVENT_CANCEL) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_CANCELLED;
            }
            break;
        case BC2_DEVICE_TRANSACTION_REVIEW:
            if (event == BC2_DEVICE_EVENT_CONFIRM) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED;
            } else if (event == BC2_DEVICE_EVENT_CANCEL) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_CANCELLED;
            }
            break;
        case BC2_DEVICE_SETTINGS:
            if (event == BC2_DEVICE_EVENT_CONFIRM) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_SETTINGS_CONFIRMED;
            } else if (event == BC2_DEVICE_EVENT_CANCEL) {
                machine->state = BC2_DEVICE_DASHBOARD;
                machine->last_action = BC2_DEVICE_ACTION_CANCELLED;
            }
            break;
        case BC2_DEVICE_ERROR:
            if (event == BC2_DEVICE_EVENT_RECOVER) lock_machine(machine, now_ms);
            break;
        default:
            break;
    }
    return machine->state != previous || machine->last_action != BC2_DEVICE_ACTION_NONE;
}

int bc2_device_machine_tick(bc2_device_machine *machine, uint64_t now_ms) {
    if (machine == 0) return 0;
    if (machine->state == BC2_DEVICE_COOLDOWN && now_ms >= machine->cooldown_until_ms) {
        return bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_COOLDOWN_EXPIRED, now_ms);
    }
    if (state_is_unlocked(machine->state) && machine->session_timeout_ms > 0U &&
        now_ms >= machine->last_activity_ms && now_ms - machine->last_activity_ms >= machine->session_timeout_ms) {
        return bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_SESSION_TIMEOUT, now_ms);
    }
    return 0;
}

uint64_t bc2_device_machine_cooldown_remaining(const bc2_device_machine *machine, uint64_t now_ms) {
    if (machine == 0 || machine->state != BC2_DEVICE_COOLDOWN || now_ms >= machine->cooldown_until_ms) return 0U;
    return machine->cooldown_until_ms - now_ms;
}

int bc2_device_machine_is_unlocked(const bc2_device_machine *machine) {
    return machine != 0 && state_is_unlocked(machine->state);
}

const char *bc2_device_state_name(bc2_device_state state) {
    switch (state) {
        case BC2_DEVICE_BOOT: return "Boot";
        case BC2_DEVICE_SETUP_REQUIRED: return "Setup required";
        case BC2_DEVICE_LOCKED: return "Locked";
        case BC2_DEVICE_UNLOCKING: return "Unlocking";
        case BC2_DEVICE_COOLDOWN: return "Cooldown";
        case BC2_DEVICE_DASHBOARD: return "Dashboard";
        case BC2_DEVICE_RECEIVE_REVIEW: return "Receive review";
        case BC2_DEVICE_TRANSACTION_REVIEW: return "Transaction review";
        case BC2_DEVICE_SETTINGS: return "Settings";
        case BC2_DEVICE_ERROR: return "Error";
        default: return "Unknown";
    }
}

const char *bc2_device_action_name(bc2_device_action action) {
    switch (action) {
        case BC2_DEVICE_ACTION_NONE: return "None";
        case BC2_DEVICE_ACTION_UNLOCKED: return "Unlocked";
        case BC2_DEVICE_ACTION_LOCKED: return "Locked";
        case BC2_DEVICE_ACTION_RECEIVE_CONFIRMED: return "Receive confirmed";
        case BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED: return "Transaction confirmed";
        case BC2_DEVICE_ACTION_SETTINGS_CONFIRMED: return "Settings confirmed";
        case BC2_DEVICE_ACTION_CANCELLED: return "Cancelled";
        case BC2_DEVICE_ACTION_COOLDOWN_STARTED: return "Cooldown started";
        case BC2_DEVICE_ACTION_FATAL_ERROR: return "Fatal error";
        default: return "Unknown";
    }
}
