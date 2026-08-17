#ifndef BC2_PIN_ENTRY_H
#define BC2_PIN_ENTRY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BC2_PIN_LENGTH 4U
#define BC2_PIN_KEY_COUNT 11U

typedef enum {
    BC2_PIN_ENTRY_CHANGED = 0,
    BC2_PIN_ENTRY_COMPLETE,
    BC2_PIN_ENTRY_IGNORED
} bc2_pin_entry_result_t;

typedef struct {
    unsigned int selected_key;
    char digits[BC2_PIN_LENGTH + 1U];
    size_t digit_count;
} bc2_pin_entry_t;

void bc2_pin_entry_init(bc2_pin_entry_t *entry);
void bc2_pin_entry_move_next(bc2_pin_entry_t *entry);
void bc2_pin_entry_move_previous(bc2_pin_entry_t *entry);
bc2_pin_entry_result_t bc2_pin_entry_confirm(bc2_pin_entry_t *entry);
int bc2_pin_entry_matches(const bc2_pin_entry_t *entry, const char *expected_pin);
void bc2_pin_entry_clear(bc2_pin_entry_t *entry);

#ifdef __cplusplus
}
#endif

#endif
