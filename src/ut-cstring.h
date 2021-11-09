#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma once

char *ut_cstring_new(const char *value);

char *ut_cstring_new_printf(const char *format, ...)
    __attribute((format(printf, 1, 2)));

char *ut_cstring_new_vprintf(const char *format, va_list ap);

void ut_cstring_set(char **string, const char *value);

void ut_cstring_clear(char **string);

char *ut_cstring_take(char **string);

#define ut_cstring_ref char *__attribute__((__cleanup__(ut_cstring_clear)))

bool ut_cstring_starts_with(const char *value, const char *prefix);

bool ut_cstring_ends_with(const char *value, const char *suffix);

char *ut_cstring_join(const char *separator, const char *value0, ...);

char *ut_cstring_substring(const char *value, size_t start, size_t end);
