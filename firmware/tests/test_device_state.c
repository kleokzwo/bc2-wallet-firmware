#include "bc2_device_state.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"CHECK failed: %s at line %d\n", #x, __LINE__); return EXIT_FAILURE; } } while (0)

static int unlock_success_flow(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 1, 1000U);
    CHECK(m.state == BC2_DEVICE_BOOT);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1001U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 1002U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_UNLOCK_SUCCESS, 1003U));
    CHECK(m.state == BC2_DEVICE_DASHBOARD);
    CHECK(m.last_action == BC2_DEVICE_ACTION_UNLOCKED);
    CHECK(bc2_device_machine_is_unlocked(&m));
    return 1;
}

static int review_and_lock_flows(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 1, 0U);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 2U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_UNLOCK_SUCCESS, 3U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_OPEN_RECEIVE, 4U));
    CHECK(m.state == BC2_DEVICE_RECEIVE_REVIEW);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_CONFIRM, 5U));
    CHECK(m.last_action == BC2_DEVICE_ACTION_RECEIVE_CONFIRMED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_OPEN_TRANSACTION, 6U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_CANCEL, 7U));
    CHECK(m.last_action == BC2_DEVICE_ACTION_CANCELLED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_LOCK, 8U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    return 1;
}

static int cooldown_flow(void) {
    bc2_device_machine m;
    unsigned int i;
    bc2_device_machine_init(&m, 1, 0U);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    for (i = 0U; i < 5U; ++i) {
        CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 10U + i * 2U));
        CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_UNLOCK_FAILURE, 11U + i * 2U));
    }
    CHECK(m.state == BC2_DEVICE_COOLDOWN);
    CHECK(m.last_action == BC2_DEVICE_ACTION_COOLDOWN_STARTED);
    CHECK(bc2_device_machine_cooldown_remaining(&m, 20U) > 0U);
    CHECK(!bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 20U));
    CHECK(!bc2_device_machine_tick(&m, m.cooldown_until_ms - 1U));
    CHECK(bc2_device_machine_tick(&m, m.cooldown_until_ms));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    CHECK(m.failed_unlock_attempts == 0U);
    return 1;
}

static int timeout_flow(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 1, 0U);
    m.session_timeout_ms = 1000U;
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 2U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_UNLOCK_SUCCESS, 100U));
    CHECK(!bc2_device_machine_tick(&m, 1099U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_ACTIVITY, 800U));
    CHECK(!bc2_device_machine_tick(&m, 1799U));
    CHECK(bc2_device_machine_tick(&m, 1800U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    return 1;
}

static int setup_and_error_flow(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 0, 0U);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    CHECK(m.state == BC2_DEVICE_SETUP_REQUIRED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_SETUP_COMPLETE, 2U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_FATAL_ERROR, 3U));
    CHECK(m.state == BC2_DEVICE_ERROR);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_RECOVER, 4U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    return 1;
}

static int factory_reset_to_setup_flow(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 1, 0U);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 1U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 2U));
    CHECK(m.state == BC2_DEVICE_UNLOCKING);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_FACTORY_RESET_TO_SETUP, 3U));
    CHECK(m.state == BC2_DEVICE_SETUP_REQUIRED);
    CHECK(m.wallet_is_initialized == 0);
    CHECK(!bc2_device_machine_is_unlocked(&m));
    return 1;
}

static int timeout_then_unlock_flow(void) {
    bc2_device_machine m;
    bc2_device_machine_init(&m, 1, 100U);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BOOT_COMPLETE, 101U));
    CHECK(m.state == BC2_DEVICE_LOCKED);
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_BEGIN_UNLOCK, 102U));
    CHECK(bc2_device_machine_dispatch(&m, BC2_DEVICE_EVENT_UNLOCK_SUCCESS, 103U));
    CHECK(m.state == BC2_DEVICE_DASHBOARD);

    CHECK(bc2_device_machine_tick(
        &m, 103U + m.session_timeout_ms + 1U));
    CHECK(m.state == BC2_DEVICE_LOCKED);

    CHECK(bc2_device_machine_dispatch(
        &m, BC2_DEVICE_EVENT_BEGIN_UNLOCK,
        103U + m.session_timeout_ms + 2U));
    CHECK(m.state == BC2_DEVICE_UNLOCKING);

    CHECK(bc2_device_machine_dispatch(
        &m, BC2_DEVICE_EVENT_UNLOCK_SUCCESS,
        103U + m.session_timeout_ms + 3U));
    CHECK(m.state == BC2_DEVICE_DASHBOARD);
    CHECK(m.last_action == BC2_DEVICE_ACTION_UNLOCKED);
    return 1;
}

int main(void) {
    CHECK(unlock_success_flow());
    CHECK(review_and_lock_flows());
    CHECK(cooldown_flow());
    CHECK(timeout_flow());
    CHECK(setup_and_error_flow());
    CHECK(factory_reset_to_setup_flow());
    CHECK(timeout_then_unlock_flow());
    return EXIT_SUCCESS;
}
