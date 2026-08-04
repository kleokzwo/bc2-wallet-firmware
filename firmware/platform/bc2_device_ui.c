#include "bc2_device_ui.h"

#include <stdio.h>
#include <string.h>

static void copy_text(char *destination, size_t capacity, const char *source) {
    if (capacity == 0U) return;
    if (source == NULL) source = "";
    (void)snprintf(destination, capacity, "%s", source);
}

bc2_hal_result_t bc2_device_ui_render(const bc2_hal_t *hal, bc2_device_screen_t screen,
                                      const char *primary_text, const char *secondary_text) {
    bc2_display_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.require_full_refresh = false;

    switch (screen) {
        case BC2_DEVICE_SCREEN_BOOT:
            copy_text(frame.title, sizeof(frame.title), "BC2 COLD WALLET");
            copy_text(frame.body, sizeof(frame.body), "Starte sicheren Wallet-Core");
            frame.require_full_refresh = true;
            break;
        case BC2_DEVICE_SCREEN_LOCKED:
            copy_text(frame.title, sizeof(frame.title), "GERAET GESPERRT");
            copy_text(frame.body, sizeof(frame.body), "USB VERBUNDEN  V0.29.7");
            copy_text(frame.footer, sizeof(frame.footer), "BEIDE: ENTSPERREN");
            break;
        case BC2_DEVICE_SCREEN_DASHBOARD:
            copy_text(frame.title, sizeof(frame.title), "BC2 DASHBOARD");
            copy_text(frame.body, sizeof(frame.body), primary_text);
            copy_text(frame.footer, sizeof(frame.footer), "Links: Empfang  Rechts: Menue");
            break;
        case BC2_DEVICE_SCREEN_RECEIVE_REVIEW:
            copy_text(frame.title, sizeof(frame.title), "ADRESSE PRUEFEN");
            copy_text(frame.body, sizeof(frame.body), primary_text);
            copy_text(frame.footer, sizeof(frame.footer), "Zurueck / Bestaetigen");
            frame.require_full_refresh = true;
            break;
        case BC2_DEVICE_SCREEN_TRANSACTION_SUMMARY:
            copy_text(frame.title, sizeof(frame.title), "TRANSAKTION PRUEFEN");
            copy_text(frame.body, sizeof(frame.body), primary_text);
            copy_text(frame.footer, sizeof(frame.footer), secondary_text);
            frame.require_full_refresh = true;
            break;
        case BC2_DEVICE_SCREEN_ERROR:
            copy_text(frame.title, sizeof(frame.title), "SICHERHEITSFEHLER");
            copy_text(frame.body, sizeof(frame.body), primary_text);
            copy_text(frame.footer, sizeof(frame.footer), "Vorgang abgebrochen");
            frame.require_full_refresh = true;
            break;
        default:
            return BC2_HAL_ERROR_ARGUMENT;
    }
    return bc2_hal_present(hal, &frame);
}

bc2_hal_result_t bc2_device_ui_render_pin(const bc2_hal_t *hal,
                                          unsigned int selected_key,
                                          unsigned int digit_count) {
    return bc2_device_ui_render_pin_prompt(hal, "PIN EINGEBEN", selected_key, digit_count);
}

bc2_hal_result_t bc2_device_ui_render_pin_prompt(const bc2_hal_t *hal,
                                                 const char *title,
                                                 unsigned int selected_key,
                                                 unsigned int digit_count) {
    bc2_display_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    copy_text(frame.title, sizeof(frame.title), title);
    (void)snprintf(frame.body, sizeof(frame.body), "%u:%u", selected_key, digit_count);
    copy_text(frame.footer, sizeof(frame.footer), "BEIDE: AUSWAEHLEN");
    return bc2_hal_present(hal, &frame);
}
