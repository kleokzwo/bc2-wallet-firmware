#include "bc2_pin_entry.h"

#include <string.h>

/* Key order: 1 2 3 / 4 5 6 / 7 8 9 / < 0 */
static const char k_keys[BC2_PIN_KEY_COUNT] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '<', '0'
};

void bc2_pin_entry_init(bc2_pin_entry_t *entry) {
    if (entry == NULL) return;
    memset(entry, 0, sizeof(*entry));
}

void bc2_pin_entry_move_next(bc2_pin_entry_t *entry) {
    if (entry == NULL) return;
    entry->selected_key = (entry->selected_key + 1U) % BC2_PIN_KEY_COUNT;
}

void bc2_pin_entry_move_previous(bc2_pin_entry_t *entry) {
    if (entry == NULL) return;
    entry->selected_key = entry->selected_key == 0U
        ? BC2_PIN_KEY_COUNT - 1U
        : entry->selected_key - 1U;
}

bc2_pin_entry_result_t bc2_pin_entry_confirm(bc2_pin_entry_t *entry) {
    char key;
    if (entry == NULL || entry->selected_key >= BC2_PIN_KEY_COUNT)
        return BC2_PIN_ENTRY_IGNORED;

    key = k_keys[entry->selected_key];
    if (key == '<') {
        if (entry->digit_count == 0U) return BC2_PIN_ENTRY_IGNORED;
        --entry->digit_count;
        entry->digits[entry->digit_count] = '\0';
        return BC2_PIN_ENTRY_CHANGED;
    }
    if (entry->digit_count >= BC2_PIN_LENGTH) return BC2_PIN_ENTRY_IGNORED;

    entry->digits[entry->digit_count++] = key;
    entry->digits[entry->digit_count] = '\0';
    return entry->digit_count == BC2_PIN_LENGTH
        ? BC2_PIN_ENTRY_COMPLETE
        : BC2_PIN_ENTRY_CHANGED;
}

int bc2_pin_entry_matches(const bc2_pin_entry_t *entry, const char *expected_pin) {
    unsigned char difference = 0U;
    size_t index;
    if (entry == NULL || expected_pin == NULL ||
        entry->digit_count != BC2_PIN_LENGTH ||
        strlen(expected_pin) != BC2_PIN_LENGTH) return 0;

    for (index = 0U; index < BC2_PIN_LENGTH; ++index)
        difference |= (unsigned char)(entry->digits[index] ^ expected_pin[index]);
    return difference == 0U;
}

void bc2_pin_entry_clear(bc2_pin_entry_t *entry) {
    volatile char *cursor;
    size_t remaining;
    if (entry == NULL) return;
    cursor = entry->digits;
    remaining = sizeof(entry->digits);
    while (remaining-- > 0U) *cursor++ = '\0';
    entry->digit_count = 0U;
}
