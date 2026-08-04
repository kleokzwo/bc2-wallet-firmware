#include "bc2_pin_entry.h"

#include <assert.h>

static void select_key(bc2_pin_entry_t *entry, unsigned int key) {
    while (entry->selected_key != key) bc2_pin_entry_move_next(entry);
}

int main(void) {
    bc2_pin_entry_t entry;
    bc2_pin_entry_init(&entry);

    select_key(&entry, 1U); /* 2 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 3U); /* 4 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 9U); /* < */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    assert(entry.digit_count == 1U);

    select_key(&entry, 3U); /* 4 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 5U); /* 6 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 7U); /* 8 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 0U); /* 1 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_CHANGED);
    select_key(&entry, 2U); /* 3 */
    assert(bc2_pin_entry_confirm(&entry) == BC2_PIN_ENTRY_COMPLETE);
    assert(bc2_pin_entry_matches(&entry, "246813"));
    assert(!bc2_pin_entry_matches(&entry, "123456"));

    bc2_pin_entry_clear(&entry);
    assert(entry.digit_count == 0U);
    return 0;
}
