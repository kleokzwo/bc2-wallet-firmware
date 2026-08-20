#include "bc2_radio_policy.h"
#include "bc2_device_flow.h"
#include "bc2_button_adapter.h"
#include "bc2_device_service.h"
#include "bc2_device_state.h"
#include "bc2_hal.h"
#include "bc2_navigation.h"
#include "bc2_pin_entry.h"
#include "bc2_pin_security.h"
#include "bc2_hw_wallet.h"
#include "bc2_bip39_words.h"
#include "waveshare_bsp.h"

#include "esp_log.h"
#include "esp_system.h"
#include "mbedtls/md.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <string.h>

#define BC2_DEVICE_LOOP_DELAY_MS 10U
#define BC2_APPLICATION_TASK_STACK_SIZE 16384U
#define BC2_APPLICATION_TASK_PRIORITY 5U
static const char *TAG = "bc2";

typedef enum {
    BC2_PIN_MODE_UNLOCK = 0,
    BC2_PIN_MODE_CREATE,
    BC2_PIN_MODE_CONFIRM,
    BC2_PIN_MODE_AUTHORIZE_RECEIVE,
    BC2_PIN_MODE_AUTHORIZE_TRANSACTION
} bc2_pin_mode_t;

typedef enum {
    BC2_PIN_POST_NONE = 0,
    BC2_PIN_POST_CREATE_WALLET,
    BC2_PIN_POST_RECOVERY
} bc2_pin_post_action_t;

typedef struct {
    bc2_pin_security_t security;
    bc2_pin_mode_t mode;
    bc2_pin_post_action_t post_action;
    char first_pin[BC2_SECURITY_PIN_LENGTH + 1U];
    char receive_address[BC2_DEVICE_RECEIVE_ADDRESS_MAX];
    uint32_t receive_index;
    bc2_device_transaction_review_t transaction;
    bool active;
} bc2_pin_session_t;

typedef enum {
    BC2_WALLET_SETUP_CONFIRM = 0,
    BC2_WALLET_SETUP_WORDS
} bc2_wallet_setup_stage_t;

typedef struct {
    bool active;
    bc2_wallet_setup_stage_t stage;
    unsigned int page;
    char words[BC2_HW_WALLET_WORD_COUNT][BC2_HW_WALLET_WORD_SIZE];
} bc2_wallet_setup_session_t;

typedef enum {
    BC2_RECOVERY_WAITING_DESKTOP = 0,
    BC2_RECOVERY_CONFIRM_IMPORT,
    BC2_RECOVERY_INVALID
} bc2_recovery_stage_t;

typedef struct {
    bool active;
    bool replacing_existing;
    bc2_recovery_stage_t stage;
    size_t word_count;
    uint16_t indexes[BC2_HW_WALLET_MAX_WORD_COUNT];
    char fingerprint[10];
} bc2_recovery_session_t;

static void clear_pin_text(char *pin) {
    volatile char *cursor = pin;
    size_t remaining = BC2_SECURITY_PIN_LENGTH + 1U;
    while (remaining-- > 0U) *cursor++ = '\0';
}

static void render_pin(const bc2_hal_t *hal, const bc2_pin_entry_t *entry,
                       bc2_pin_mode_t mode) {
    const char *title = mode == BC2_PIN_MODE_CREATE ? "PIN ANLEGEN" :
                        mode == BC2_PIN_MODE_CONFIRM ? "PIN WIEDERHOLEN" :
                        mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE ? "EMPFANG FREIGEBEN" :
                        mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION ? "ZAHLUNG FREIGEBEN" :
                        "PIN EINGEBEN";
    (void)bc2_device_ui_render_pin_prompt(hal, title, entry->selected_key,
                                         (unsigned int)entry->digit_count);
}

static void render_current_state(const bc2_hal_t *hal,
                                 const bc2_device_machine *machine) {
    char status[160];
    bc2_device_view_data_t view;

    if (machine->state == BC2_DEVICE_SETUP_REQUIRED) {
        (void)snprintf(status, sizeof(status),
                       "NOCH KEINE WALLET\nFirmware %s",
                       BC2_DEVICE_FIRMWARE_VERSION);
        view.primary_text = status;
        view.secondary_text = "Desktop: Neue Wallet erstellen";
    } else {
        (void)snprintf(status,
                       sizeof(status),
                       "Firmware %s\n%s\nDisplay %ux%u",
                       BC2_DEVICE_FIRMWARE_VERSION,
                       BC2_BOARD_NAME,
                       BC2_BOARD_DISPLAY_WIDTH,
                       BC2_BOARD_DISPLAY_HEIGHT);
        view.primary_text = status;
        view.secondary_text = "BC2 Hardware Wallet";
    }

    if (bc2_device_flow_render(hal, machine, &view) != BC2_HAL_OK)
        ESP_LOGE(TAG, "Unable to render device state");
}

static void render_wallet_setup_confirm(const bc2_hal_t *hal) {
    bc2_display_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    (void)snprintf(frame.title, sizeof(frame.title), "NEUE WALLET");
    (void)snprintf(frame.body, sizeof(frame.body),
                   "Seed wird NUR auf\ndiesem Geraet erzeugt.");
    (void)snprintf(frame.footer, sizeof(frame.footer),
                   "BEIDE: ERSTELLEN  LINKS: ABBRUCH");
    frame.require_full_refresh = true;
    (void)bc2_hal_present(hal, &frame);
}

static void render_recovery_page(const bc2_hal_t *hal,
                                 const bc2_wallet_setup_session_t *setup) {
    bc2_display_frame_t frame;
    const unsigned int first = setup->page * 3U;
    memset(&frame, 0, sizeof(frame));
    (void)snprintf(frame.title, sizeof(frame.title), "RECOVERY %u/4", setup->page + 1U);
    (void)snprintf(frame.body, sizeof(frame.body),
                   "%u %s\n%u %s\n%u %s",
                   first + 1U, setup->words[first],
                   first + 2U, setup->words[first + 1U],
                   first + 3U, setup->words[first + 2U]);
    (void)snprintf(frame.footer, sizeof(frame.footer),
                   setup->page == 3U ? "BEIDE: BACKUP BESTAETIGT" : "BEIDE: WEITER");
    frame.require_full_refresh = true;
    (void)bc2_hal_present(hal, &frame);
}

static void render_recovery(const bc2_hal_t *hal, const bc2_recovery_session_t *recovery) {
    bc2_display_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    (void)snprintf(frame.title, sizeof(frame.title), "RECOVERY");
    if (recovery->stage == BC2_RECOVERY_INVALID) {
        (void)snprintf(frame.body, sizeof(frame.body), "SEED UNGUELTIG");
        (void)snprintf(frame.footer, sizeof(frame.footer), "DESKTOP PRUEFEN");
    } else {
        (void)snprintf(frame.body, sizeof(frame.body), "EINGABE AM DESKTOP");
        (void)snprintf(frame.footer, sizeof(frame.footer), "WARTE...");
    }
    frame.require_full_refresh = true;
    (void)bc2_hal_present(hal, &frame);
}

static void recovery_clear(bc2_recovery_session_t *recovery) {
    if (recovery == NULL) return;
    volatile uint16_t *p = recovery->indexes;
    for (size_t i=0; i<BC2_HW_WALLET_MAX_WORD_COUNT; ++i) p[i]=0U;
    memset(recovery->fingerprint, 0, sizeof(recovery->fingerprint));
    recovery->active=false;
    recovery->replacing_existing=false;
    recovery->stage=BC2_RECOVERY_WAITING_DESKTOP;
    recovery->word_count=0U;
}

static void wallet_setup_abort(bc2_wallet_setup_session_t *setup) {
    if (setup == NULL) return;
    bc2_hw_wallet_clear_words(setup->words);
    setup->active = false;
    setup->page = 0U;
    setup->stage = BC2_WALLET_SETUP_CONFIRM;
}

static void process_button(const bc2_hal_t *hal,
                           bc2_device_machine *machine,
                           bc2_device_service_t *service,
                           bc2_navigation_t *navigation,
                           bc2_pin_entry_t *pin_entry,
                           bc2_pin_session_t *pin_session,
                           bc2_wallet_setup_session_t *wallet_setup,
                           bc2_recovery_session_t *recovery) {
    bc2_button_event_t raw_event;
    bc2_button_event_t button_event;
    bc2_device_event device_event;
    const bc2_hal_result_t result = bc2_button_adapter_poll(&raw_event);

    if (result == BC2_HAL_ERROR_UNAVAILABLE || result == BC2_HAL_ERROR_NOT_FOUND) return;
    if (result != BC2_HAL_OK) {
        ESP_LOGW(TAG, "Button polling failed: %d", (int)result);
        return;
    }

    if (!bc2_navigation_process(navigation, &raw_event, &button_event)) return;

    if (recovery->active) {
        if (button_event.button == BC2_BUTTON_LEFT || button_event.button == BC2_BUTTON_BACK) {
            bc2_device_service_cancel_recovery(service);
            recovery_clear(recovery);
            render_current_state(hal, machine);
            return;
        }
        render_recovery(hal, recovery);
        return;
    }

    if (wallet_setup->active) {
        if (wallet_setup->stage == BC2_WALLET_SETUP_CONFIRM) {
            if (button_event.button == BC2_BUTTON_LEFT || button_event.button == BC2_BUTTON_BACK) {
                wallet_setup_abort(wallet_setup);
                render_current_state(hal, machine);
                return;
            }
            if (button_event.button == BC2_BUTTON_CONFIRM) {
                const bc2_hw_wallet_status_t current = bc2_hw_wallet_status(hal);
                bool ok = false;
                if (current == BC2_HW_WALLET_NONE)
                    ok = bc2_hw_wallet_create(hal, wallet_setup->words);
                else if (current == BC2_HW_WALLET_BACKUP_PENDING)
                    ok = bc2_hw_wallet_load_words(hal, wallet_setup->words);
                if (!ok) {
                    wallet_setup_abort(wallet_setup);
                    (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                                                      bc2_hal_now_ms(hal));
                    render_current_state(hal, machine);
                    return;
                }
                wallet_setup->stage = BC2_WALLET_SETUP_WORDS;
                wallet_setup->page = 0U;
                render_recovery_page(hal, wallet_setup);
                return;
            }
            render_wallet_setup_confirm(hal);
            return;
        }

        if (wallet_setup->stage == BC2_WALLET_SETUP_WORDS) {
            if (button_event.button == BC2_BUTTON_LEFT && wallet_setup->page > 0U) {
                --wallet_setup->page;
                render_recovery_page(hal, wallet_setup);
                return;
            }
            if (button_event.button == BC2_BUTTON_RIGHT && wallet_setup->page < 3U) {
                ++wallet_setup->page;
                render_recovery_page(hal, wallet_setup);
                return;
            }
            if (button_event.button == BC2_BUTTON_CONFIRM) {
                if (wallet_setup->page < 3U) {
                    ++wallet_setup->page;
                    render_recovery_page(hal, wallet_setup);
                    return;
                }
                if (!bc2_hw_wallet_confirm_backup(hal)) {
                    wallet_setup_abort(wallet_setup);
                    (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                                                      bc2_hal_now_ms(hal));
                    render_current_state(hal, machine);
                    return;
                }
                wallet_setup_abort(wallet_setup);
                (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_SETUP_COMPLETE,
                                                  bc2_hal_now_ms(hal));
                render_current_state(hal, machine);
                return;
            }
            render_recovery_page(hal, wallet_setup);
            return;
        }
    }

    if (pin_session->active) {
        if (button_event.button == BC2_BUTTON_BACK &&
            (pin_session->mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE ||
             pin_session->mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION)) {
            if (pin_session->mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE)
                bc2_device_service_complete_receive(service, 0, NULL);
            else
                bc2_device_service_complete_transaction(service, 0);
            bc2_pin_entry_clear(pin_entry);
            memset(pin_session->receive_address, 0,
                   sizeof(pin_session->receive_address));
            memset(&pin_session->transaction, 0,
                   sizeof(pin_session->transaction));
            pin_session->receive_index = 0U;
            pin_session->active = false;
            render_current_state(hal, machine);
            return;
        }
        if (button_event.button == BC2_BUTTON_RIGHT) {
            bc2_pin_entry_move_next(pin_entry);
        } else if (button_event.button == BC2_BUTTON_LEFT) {
            bc2_pin_entry_move_previous(pin_entry);
        } else if (button_event.button == BC2_BUTTON_CONFIRM) {
            const bc2_pin_entry_result_t pin_result = bc2_pin_entry_confirm(pin_entry);
            if (pin_result == BC2_PIN_ENTRY_COMPLETE) {
                bc2_device_event unlock_result = BC2_DEVICE_EVENT_UNLOCK_FAILURE;
                const uint64_t now_ms = bc2_hal_now_ms(hal);
                if (pin_session->mode == BC2_PIN_MODE_CREATE) {
                    (void)snprintf(pin_session->first_pin, sizeof(pin_session->first_pin),
                                   "%s", pin_entry->digits);
                    pin_session->mode = BC2_PIN_MODE_CONFIRM;
                    bc2_pin_entry_clear(pin_entry);
                    bc2_pin_entry_init(pin_entry);
                    render_pin(hal, pin_entry, pin_session->mode);
                    return;
                }
                if (pin_session->mode == BC2_PIN_MODE_CONFIRM) {
                    if (bc2_pin_entry_matches(pin_entry, pin_session->first_pin) &&
                        bc2_pin_security_create(&pin_session->security, hal,
                                                pin_entry->digits) == BC2_PIN_SECURITY_OK)
                        unlock_result = BC2_DEVICE_EVENT_UNLOCK_SUCCESS;
                    else
                        pin_session->mode = BC2_PIN_MODE_CREATE;
                    clear_pin_text(pin_session->first_pin);
                } else if (pin_session->mode == BC2_PIN_MODE_AUTHORIZE_RECEIVE ||
                           pin_session->mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION) {
                    const bc2_pin_security_result_t verified = bc2_pin_security_verify(
                        &pin_session->security, hal, pin_entry->digits, now_ms);
                    bc2_pin_entry_clear(pin_entry);
                    if (verified == BC2_PIN_SECURITY_OK) {
                        char transaction_text[256];
                        const bool is_transaction =
                            pin_session->mode == BC2_PIN_MODE_AUTHORIZE_TRANSACTION;
                        if (is_transaction) {
                            (void)snprintf(transaction_text, sizeof(transaction_text),
                                           "AN: %s\nBETRAG: %llu sat\nGEBUEHR: %llu sat\nWECHSEL: %llu sat",
                                           pin_session->transaction.recipient_address,
                                           (unsigned long long)pin_session->transaction.recipient_amount,
                                           (unsigned long long)pin_session->transaction.fee_amount,
                                           (unsigned long long)pin_session->transaction.change_amount);
                        }
                        bc2_device_view_data_t view = {
                            is_transaction ? transaction_text : pin_session->receive_address,
                            "BEIDE: OK  BOOT: ABBRUCH"};
                        pin_session->active = false;
                        (void)bc2_device_machine_dispatch(machine,
                                                          is_transaction
                                                              ? BC2_DEVICE_EVENT_OPEN_TRANSACTION
                                                              : BC2_DEVICE_EVENT_OPEN_RECEIVE,
                                                          now_ms);
                        (void)bc2_device_flow_render(hal, machine, &view);
                    } else {
                        bc2_pin_entry_init(pin_entry);
                        render_pin(hal, pin_entry, pin_session->mode);
                    }
                    return;
                } else {
                    const bc2_pin_security_result_t verified = bc2_pin_security_verify(
                        &pin_session->security, hal, pin_entry->digits, now_ms);
                    if (verified == BC2_PIN_SECURITY_OK)
                        unlock_result = BC2_DEVICE_EVENT_UNLOCK_SUCCESS;
                    else if (verified == BC2_PIN_SECURITY_DELAYED)
                        ESP_LOGW(TAG, "PIN delayed for %llu ms",
                                 (unsigned long long)bc2_pin_security_remaining_delay(
                                     &pin_session->security, now_ms));
                }
                bc2_pin_entry_clear(pin_entry);

                if (pin_session->mode == BC2_PIN_MODE_UNLOCK &&
                    pin_session->post_action == BC2_PIN_POST_RECOVERY) {
                    if (unlock_result == BC2_DEVICE_EVENT_UNLOCK_SUCCESS) {
                        pin_session->post_action = BC2_PIN_POST_NONE;
                        if (!bc2_hw_wallet_factory_reset(hal) ||
                            !bc2_hw_wallet_restore_indexes(hal, recovery->indexes,
                                                           recovery->word_count)) {
                            bc2_device_service_cancel_recovery(service);
                            recovery_clear(recovery);
                            pin_session->active = false;
                            (void)bc2_device_machine_dispatch(machine,
                                                              BC2_DEVICE_EVENT_FATAL_ERROR,
                                                              now_ms);
                            render_current_state(hal, machine);
                            return;
                        }
                        bc2_device_service_cancel_recovery(service);
                        recovery_clear(recovery);
                        /* Keep the verified existing PIN. The machine is already
                         * in UNLOCKING, so the normal UNLOCK_SUCCESS dispatch below
                         * opens the recovered wallet. */
                    } else {
                        pin_session->post_action = BC2_PIN_POST_NONE;
                        bc2_device_service_cancel_recovery(service);
                        recovery_clear(recovery);
                    }
                }

                if (pin_session->mode == BC2_PIN_MODE_CONFIRM &&
                    unlock_result == BC2_DEVICE_EVENT_UNLOCK_SUCCESS &&
                    machine->state == BC2_DEVICE_SETUP_REQUIRED) {
                    pin_session->active = false;
                    if (pin_session->post_action == BC2_PIN_POST_CREATE_WALLET) {
                        pin_session->post_action = BC2_PIN_POST_NONE;
                        wallet_setup->active = true;
                        wallet_setup->stage = BC2_WALLET_SETUP_CONFIRM;
                        wallet_setup->page = 0U;
                        render_wallet_setup_confirm(hal);
                    } else if (pin_session->post_action == BC2_PIN_POST_RECOVERY) {
                        pin_session->post_action = BC2_PIN_POST_NONE;
                        if (!bc2_hw_wallet_restore_indexes(hal, recovery->indexes,
                                                           recovery->word_count)) {
                            bc2_device_service_cancel_recovery(service);
                            recovery_clear(recovery);
                            (void)bc2_device_machine_dispatch(machine,
                                                              BC2_DEVICE_EVENT_FATAL_ERROR,
                                                              now_ms);
                            render_current_state(hal, machine);
                            return;
                        }
                        bc2_device_service_cancel_recovery(service);
                        recovery_clear(recovery);
                        (void)bc2_device_machine_dispatch(machine,
                                                          BC2_DEVICE_EVENT_SETUP_COMPLETE,
                                                          now_ms);
                        (void)bc2_device_machine_dispatch(machine,
                                                          BC2_DEVICE_EVENT_BEGIN_UNLOCK,
                                                          now_ms);
                        (void)bc2_device_machine_dispatch(machine,
                                                          BC2_DEVICE_EVENT_UNLOCK_SUCCESS,
                                                          now_ms);
                        render_current_state(hal, machine);
                    } else render_current_state(hal, machine);
                    return;
                }
                (void)bc2_device_machine_dispatch(machine, unlock_result, now_ms);
                if (machine->state == BC2_DEVICE_UNLOCKING) {
                    bc2_pin_entry_init(pin_entry);
                    render_pin(hal, pin_entry, pin_session->mode);
                } else {
                    pin_session->active = false;
                    render_current_state(hal, machine);
                }
                return;
            }
        }
        render_pin(hal, pin_entry, pin_session->mode);
        return;
    }

    const bc2_device_state state_before_button = machine->state;
    if (bc2_device_flow_event_from_button(machine->state,
                                          &button_event,
                                          &device_event) == 0)
        return;
    if (bc2_device_machine_dispatch(machine, device_event, bc2_hal_now_ms(hal)) != 0) {
        if (machine->state == BC2_DEVICE_UNLOCKING) {
            bc2_pin_entry_init(pin_entry);
            pin_session->active = true;
            render_pin(hal, pin_entry, pin_session->mode);
        } else {
            if (machine->last_action == BC2_DEVICE_ACTION_RECEIVE_CONFIRMED) {
                /* The address is released to the desktop only after physical
                 * confirmation AND durable index advancement. */
                if (bc2_hw_wallet_commit_receive_index(hal,
                                                       pin_session->receive_index)) {
                    bc2_device_service_complete_receive(
                        service, 1, pin_session->receive_address);
                } else {
                    bc2_device_service_complete_receive(service, 0, NULL);
                    (void)bc2_device_machine_dispatch(
                        machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                        bc2_hal_now_ms(hal));
                }
            } else if (machine->last_action == BC2_DEVICE_ACTION_CANCELLED &&
                       state_before_button == BC2_DEVICE_RECEIVE_REVIEW) {
                bc2_device_service_complete_receive(service, 0, NULL);
            }

            if (machine->last_action == BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED ||
                machine->last_action == BC2_DEVICE_ACTION_CANCELLED) {
                if (machine->last_action == BC2_DEVICE_ACTION_TRANSACTION_CONFIRMED)
                    bc2_device_service_complete_transaction(service, 1);
                else if (state_before_button == BC2_DEVICE_TRANSACTION_REVIEW)
                    bc2_device_service_complete_transaction(service, 0);
                memset(&pin_session->transaction, 0, sizeof(pin_session->transaction));
            }

            if (machine->last_action == BC2_DEVICE_ACTION_RECEIVE_CONFIRMED ||
                (machine->last_action == BC2_DEVICE_ACTION_CANCELLED &&
                 state_before_button == BC2_DEVICE_RECEIVE_REVIEW)) {
                memset(pin_session->receive_address, 0,
                       sizeof(pin_session->receive_address));
                pin_session->receive_index = 0U;
            }
            render_current_state(hal, machine);
        }
    }
}

static void process_create_wallet_request(bc2_device_service_t *service,
                                          const bc2_device_machine *machine,
                                          bc2_wallet_setup_session_t *wallet_setup,
                                          const bc2_hal_t *hal,
                                          bc2_pin_entry_t *pin_entry,
                                          bc2_pin_session_t *pin_session) {
    if (machine->state != BC2_DEVICE_SETUP_REQUIRED || wallet_setup->active || pin_session->active)
        return;
    if (!bc2_device_service_take_create_wallet(service)) return;
    if (!pin_session->security.configured) {
        pin_session->mode = BC2_PIN_MODE_CREATE;
        pin_session->post_action = BC2_PIN_POST_CREATE_WALLET;
        pin_session->active = true;
        bc2_pin_entry_clear(pin_entry); bc2_pin_entry_init(pin_entry);
        render_pin(hal, pin_entry, pin_session->mode);
        return;
    }
    wallet_setup->active = true; wallet_setup->stage = BC2_WALLET_SETUP_CONFIRM; wallet_setup->page = 0U;
    render_wallet_setup_confirm(hal);
}

static int recovery_word_index(const char *word, uint16_t *index) {
    if (word == NULL || index == NULL) return 0;
    for (uint16_t i = 0U; i < 2048U; ++i) {
        if (strcmp(word, bc2_bip39_english_words[i]) == 0) {
            *index = i;
            return 1;
        }
    }
    return 0;
}

static bool recovery_parse_mnemonic(char *mnemonic, bc2_recovery_session_t *recovery) {
    char *cursor = mnemonic;
    size_t count = 0U;
    if (mnemonic == NULL || recovery == NULL) return false;
    while (*cursor != '\0') {
        char *start;
        while (*cursor == ' ') ++cursor;
        if (*cursor == '\0') break;
        start = cursor;
        while (*cursor != '\0' && *cursor != ' ') ++cursor;
        if (*cursor != '\0') *cursor++ = '\0';
        if (count >= BC2_HW_WALLET_MAX_WORD_COUNT ||
            !recovery_word_index(start, &recovery->indexes[count])) return false;
        ++count;
    }
    if (count != 12U && count != 24U) return false;
    recovery->word_count = count;
    return bc2_hw_wallet_validate_indexes(recovery->indexes, recovery->word_count);
}

static bool recovery_make_fingerprint(const char *mnemonic, char output[10]) {
    const mbedtls_md_info_t *sha256 = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    uint8_t hash[32];
    static const char hex[] = "0123456789ABCDEF";
    size_t length;
    if (mnemonic == NULL || output == NULL || sha256 == NULL) return false;
    length = strlen(mnemonic);
    if (mbedtls_md(sha256, (const unsigned char *)mnemonic, length, hash) != 0) return false;
    output[0]=hex[hash[0]>>4U]; output[1]=hex[hash[0]&0x0fU];
    output[2]=hex[hash[1]>>4U]; output[3]=hex[hash[1]&0x0fU];
    output[4]=' ';
    output[5]=hex[hash[2]>>4U]; output[6]=hex[hash[2]&0x0fU];
    output[7]=hex[hash[3]>>4U]; output[8]=hex[hash[3]&0x0fU]; output[9]='\0';
    memset(hash, 0, sizeof(hash));
    return true;
}

static void process_recovery_request(bc2_device_service_t *service,
                                     bc2_device_machine *machine,
                                     const bc2_hal_t *hal,
                                     bc2_recovery_session_t *recovery,
                                     const bc2_pin_session_t *pin_session) {
    if (recovery->active || pin_session->active || !bc2_device_service_take_recovery(service)) return;
    if (machine->state != BC2_DEVICE_SETUP_REQUIRED && machine->state != BC2_DEVICE_LOCKED) return;
    recovery_clear(recovery);
    recovery->active = true;
    recovery->replacing_existing = machine->wallet_is_initialized != 0;
    recovery->stage = BC2_RECOVERY_WAITING_DESKTOP;
    /* Desktop-assisted recovery: mnemonic entry stays on the desktop.
     * The hardware remains on its current screen until it needs a PIN. */
}

static void process_recovery_mnemonic(bc2_device_service_t *service,
                                      bc2_device_machine *machine,
                                      const bc2_hal_t *hal,
                                      bc2_recovery_session_t *recovery,
                                      bc2_pin_entry_t *pin_entry,
                                      bc2_pin_session_t *pin_session) {
    char mnemonic[BC2_DEVICE_RECOVERY_MNEMONIC_MAX];

    if (!recovery->active || recovery->stage != BC2_RECOVERY_WAITING_DESKTOP ||
        pin_session->active)
        return;

    memset(mnemonic, 0, sizeof(mnemonic));
    if (!bc2_device_service_take_recovery_mnemonic(service, mnemonic, sizeof(mnemonic)))
        return;

    if (!recovery_parse_mnemonic(mnemonic, recovery)) {
        memset(mnemonic, 0, sizeof(mnemonic));
        recovery->stage = BC2_RECOVERY_INVALID;
        render_recovery(hal, recovery);
        return;
    }
    memset(mnemonic, 0, sizeof(mnemonic));

    /* v0.39.0: VALID DESKTOP MNEMONIC -> PIN DIRECTLY.
     * There is NO hardware mnemonic entry, word navigation, fingerprint screen,
     * or second seed confirmation.
     * Existing wallet: current 4-digit PIN authorizes replacement.
     * Fresh device: create/confirm a new 4-digit PIN, then restore automatically. */
    recovery->active = false;
    bc2_pin_entry_clear(pin_entry);
    bc2_pin_entry_init(pin_entry);
    pin_session->post_action = BC2_PIN_POST_RECOVERY;
    pin_session->active = true;

    if (recovery->replacing_existing) {
        pin_session->mode = BC2_PIN_MODE_UNLOCK;
        (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_BEGIN_UNLOCK,
                                          bc2_hal_now_ms(hal));
    } else {
        /* A leftover PIN without a wallet is an incomplete setup. Recovery
         * starts clean and lets the user choose a fresh PIN. */
        if (pin_session->security.configured)
            (void)bc2_pin_security_reset(&pin_session->security, hal);
        pin_session->mode = BC2_PIN_MODE_CREATE;
    }
    render_pin(hal, pin_entry, pin_session->mode);
}

static void process_unlock_request(bc2_device_service_t *service,
                                   bc2_device_machine *machine,
                                   const bc2_hal_t *hal,
                                   bc2_pin_entry_t *pin_entry,
                                   bc2_pin_session_t *pin_session) {
    if (pin_session->active || !bc2_device_service_take_unlock(service) || machine->state != BC2_DEVICE_LOCKED) return;
    (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_BEGIN_UNLOCK, bc2_hal_now_ms(hal));
    pin_session->mode = BC2_PIN_MODE_UNLOCK; pin_session->post_action = BC2_PIN_POST_NONE; pin_session->active = true;
    bc2_pin_entry_clear(pin_entry); bc2_pin_entry_init(pin_entry); render_pin(hal,pin_entry,pin_session->mode);
}

static void process_transaction_request(bc2_device_service_t *service,
                                        const bc2_hal_t *hal,
                                        const bc2_device_machine *machine,
                                        bc2_pin_entry_t *pin_entry,
                                        bc2_pin_session_t *pin_session) {
    if (machine->state != BC2_DEVICE_DASHBOARD || pin_session->active) return;
    if (!bc2_device_service_take_transaction(service, &pin_session->transaction)) return;
    pin_session->mode = BC2_PIN_MODE_AUTHORIZE_TRANSACTION;
    pin_session->active = true;
    bc2_pin_entry_clear(pin_entry);
    bc2_pin_entry_init(pin_entry);
    render_pin(hal, pin_entry, pin_session->mode);
}

static void process_sign_request(bc2_device_service_t*s,const bc2_hal_t*h,const bc2_device_machine*m){
 bc2_device_sign_request_t r;uint8_t pub[33]={0},sig[80]={0};size_t n=0;if(!s||!h||!m||m->state!=BC2_DEVICE_DASHBOARD)return;
 memset(&r,0,sizeof r);if(!bc2_device_service_take_sign_request(s,&r))return;
 int ok=bc2_hw_wallet_sign_single_p2wpkh(h,r.input_address,r.prev_txid_le,r.prev_output_index,r.input_amount,0xfffffffdU,
 s->reviewed_transaction.recipient_address,s->reviewed_transaction.recipient_amount,s->reviewed_transaction.change_amount,0U,pub,sig,sizeof sig,&n);
 bc2_device_service_complete_sign(s,ok,pub,sig,n);memset(&r,0,sizeof r);memset(pub,0,sizeof pub);memset(sig,0,sizeof sig);
}

static void process_lock_request(bc2_device_service_t *service,
                                 bc2_device_machine *machine,
                                 const bc2_hal_t *hal,
                                 bc2_pin_session_t *pin_session) {
    if (!bc2_device_service_take_lock(service)) return;

    /* Logout is a lock only: wallet/seed/PIN remain intact. */
    pin_session->active = false;
    pin_session->post_action = BC2_PIN_POST_NONE;
    memset(pin_session->receive_address, 0, sizeof(pin_session->receive_address));
    memset(&pin_session->transaction, 0, sizeof(pin_session->transaction));
    pin_session->receive_index = 0U;

    (void)bc2_device_machine_dispatch(machine, BC2_DEVICE_EVENT_LOCK,
                                      bc2_hal_now_ms(hal));
    bc2_device_service_clear_wallet_id(service);
    render_current_state(hal, machine);
}

static void process_receive_request(bc2_device_service_t *service,
                                    const bc2_hal_t *hal,
                                    const bc2_device_machine *machine,
                                    bc2_pin_entry_t *pin_entry,
                                    bc2_pin_session_t *pin_session) {
    uint32_t index = 0U;

    if (machine->state != BC2_DEVICE_DASHBOARD || pin_session->active) return;
    if (!bc2_device_service_take_receive_request(service)) return;

    memset(pin_session->receive_address, 0,
           sizeof(pin_session->receive_address));

    if (!bc2_hw_wallet_receive_index(hal, &index) ||
        !bc2_hw_wallet_receive_address(hal, index,
                                       pin_session->receive_address,
                                       sizeof(pin_session->receive_address))) {
        /* status 4 = technical derivation failure (not a user rejection) */
        service->receive_result = (bc2_device_review_result_t)4;
        service->receive_request_pending = 0;
        service->receive_review_active = 0;
        ESP_LOGE(TAG, "Unable to derive receive address on hardware");
        return;
    }

    pin_session->receive_index = index;
    pin_session->mode = BC2_PIN_MODE_AUTHORIZE_RECEIVE;
    pin_session->active = true;
    bc2_pin_entry_clear(pin_entry);
    bc2_pin_entry_init(pin_entry);
    render_pin(hal, pin_entry, pin_session->mode);
}

static void sync_authenticated_wallet_id(bc2_device_service_t *service,
                                         const bc2_hal_t *hal,
                                         const bc2_device_machine *machine) {
    uint8_t wallet_id[BC2_HW_WALLET_ID_SIZE] = {0};
    if (service == NULL || hal == NULL || machine == NULL) return;

    if (machine->state != BC2_DEVICE_DASHBOARD) {
        bc2_device_service_clear_wallet_id(service);
        return;
    }

    if (!service->wallet_id_available) {
        if (bc2_hw_wallet_id(hal, wallet_id))
            bc2_device_service_set_wallet_id(service, wallet_id);
        else
            ESP_LOGE(TAG, "Unable to derive authenticated wallet id");
    }
    memset(wallet_id, 0, sizeof(wallet_id));
}

static void process_usb(bc2_device_service_t *service,
                        const bc2_hal_t *hal,
                        const bc2_device_machine *machine) {
    uint8_t capabilities = BC2_DEVICE_CAP_USB |
                           BC2_DEVICE_CAP_STORAGE |
                           BC2_DEVICE_CAP_RANDOM;
    if (bc2_waveshare_display_ready()) capabilities |= BC2_DEVICE_CAP_DISPLAY;
    if (bc2_waveshare_buttons_ready()) capabilities |= BC2_DEVICE_CAP_BUTTONS;

    const bc2_device_identity_t identity = {
        BC2_BOARD_NAME,
        BC2_BOARD_DISPLAY_WIDTH,
        BC2_BOARD_DISPLAY_HEIGHT,
        (uint8_t)bc2_waveshare_board_revision(),
        capabilities,
    };
    const bc2_hal_result_t result = bc2_device_service_process_usb(service,
                                                                   hal,
                                                                   machine,
                                                                   &identity);
    if (result != BC2_HAL_OK &&
        result != BC2_HAL_ERROR_NOT_FOUND &&
        result != BC2_HAL_ERROR_UNAVAILABLE)
        ESP_LOGW(TAG, "USB request failed: %d", (int)result);
}

static bool random_self_test(const bc2_hal_t *hal) {
    uint8_t random_probe[32];
    return bc2_hal_random(hal, random_probe, sizeof(random_probe)) == BC2_HAL_OK;
}

static void bc2_application_task(void *argument) {
    (void)argument;
    bc2_hal_t hal = {0};
    bc2_device_machine machine;
    bc2_device_service_t device_service;
    bc2_navigation_t navigation;
    bc2_pin_entry_t pin_entry;
    bc2_pin_session_t pin_session = {0};
    bc2_wallet_setup_session_t wallet_setup = {0};
    bc2_recovery_session_t recovery = {0};

    ESP_ERROR_CHECK(bc2_waveshare_bsp_init(&hal));
    ESP_ERROR_CHECK(bc2_button_adapter_init());
    if (!bc2_hal_is_complete(&hal)) {
        ESP_LOGE(TAG, "HAL incomplete");
        esp_restart();
    }

    const bc2_hw_wallet_status_t wallet_status = bc2_hw_wallet_status(&hal);
    bc2_device_machine_init(&machine, wallet_status == BC2_HW_WALLET_READY,
                            bc2_hal_now_ms(&hal));
    bc2_device_service_init(&device_service);
    if (wallet_status == BC2_HW_WALLET_ERROR) {
        ESP_LOGE(TAG, "Wallet storage invalid; refusing to overwrite existing data");
        (void)bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                                          bc2_hal_now_ms(&hal));
    }
    bc2_navigation_init(&navigation);
    bc2_pin_entry_init(&pin_entry);
    const bc2_pin_security_result_t pin_load = bc2_pin_security_load(
        &pin_session.security, &hal, bc2_hal_now_ms(&hal));
    if (pin_load == BC2_PIN_SECURITY_NOT_CONFIGURED && wallet_status == BC2_HW_WALLET_READY) {
        ESP_LOGE(TAG, "Wallet exists but PIN is not configured");
        (void)bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_FATAL_ERROR, bc2_hal_now_ms(&hal));
    } else if (pin_load != BC2_PIN_SECURITY_OK && pin_load != BC2_PIN_SECURITY_NOT_CONFIGURED) {
        ESP_LOGE(TAG, "PIN storage invalid");
        (void)bc2_device_machine_dispatch(&machine, BC2_DEVICE_EVENT_FATAL_ERROR,
                                          bc2_hal_now_ms(&hal));
    }
    render_current_state(&hal, &machine);

    const bc2_device_event boot_result = random_self_test(&hal)
        ? BC2_DEVICE_EVENT_BOOT_COMPLETE
        : BC2_DEVICE_EVENT_FATAL_ERROR;
    (void)bc2_device_machine_dispatch(&machine,
                                      boot_result,
                                      bc2_hal_now_ms(&hal));
    render_current_state(&hal, &machine);

    ESP_LOGI(TAG,
             "v%s bring-up started; display=%s buttons=%s USB=ready Wi-Fi/BLE=off",
             BC2_DEVICE_FIRMWARE_VERSION,
             bc2_waveshare_display_ready() ? "ready" : "safety-gated",
             bc2_waveshare_buttons_ready() ? "ready" : "safety-gated");

    for (;;) {
        /* Physical input has priority and must never wait behind USB I/O. */
        process_button(&hal, &machine, &device_service, &navigation, &pin_entry,
                       &pin_session, &wallet_setup, &recovery);
        sync_authenticated_wallet_id(&device_service, &hal, &machine);
        process_usb(&device_service, &hal, &machine);
        process_lock_request(&device_service, &machine, &hal, &pin_session);
        process_create_wallet_request(&device_service, &machine, &wallet_setup,
                                      &hal, &pin_entry, &pin_session);
        process_recovery_request(&device_service, &machine, &hal, &recovery, &pin_session);
        process_recovery_mnemonic(&device_service, &machine, &hal, &recovery, &pin_entry, &pin_session);
        process_unlock_request(&device_service, &machine, &hal, &pin_entry, &pin_session);
        process_receive_request(&device_service, &hal, &machine, &pin_entry, &pin_session);
        process_transaction_request(&device_service, &hal, &machine, &pin_entry, &pin_session);
        process_sign_request(&device_service, &hal, &machine);
        if (bc2_device_machine_tick(&machine, bc2_hal_now_ms(&hal)) != 0) {
            if (machine.state != BC2_DEVICE_TRANSACTION_REVIEW)
                bc2_device_service_complete_transaction(&device_service, 0);
            render_current_state(&hal, &machine);
        }
        vTaskDelay(pdMS_TO_TICKS(BC2_DEVICE_LOOP_DELAY_MS));
    }
}

void app_main(void) {
    bc2_radio_policy_assert_disabled();
    const BaseType_t created = xTaskCreate(bc2_application_task,
                                           "bc2_application",
                                           BC2_APPLICATION_TASK_STACK_SIZE,
                                           NULL,
                                           BC2_APPLICATION_TASK_PRIORITY,
                                           NULL);
    if (created != pdPASS) {
        ESP_LOGE(TAG, "Unable to create BC2 application task");
        esp_restart();
    }
}
