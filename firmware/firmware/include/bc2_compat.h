#ifndef BC2_COMPAT_H
#define BC2_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/*
 * Keep platform-specific C library differences in one small place.
 * Wallet modules should not contain compiler-specific branches.
 */
static inline char *bc2_strtok(char *text, const char *delimiters, char **context)
{
#ifdef _WIN32
    return strtok_s(text, delimiters, context);
#else
    return strtok_r(text, delimiters, context);
#endif
}

static inline bool bc2_copy_string(char *destination,
                                   size_t destination_size,
                                   const char *source)
{
    size_t source_length;

    if (!destination || destination_size == 0U || !source) {
        return false;
    }

    source_length = strlen(source);
    if (source_length >= destination_size) {
        return false;
    }

    memcpy(destination, source, source_length + 1U);
    return true;
}

#endif
