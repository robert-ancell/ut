#include <stdarg.h>
#include <stdlib.h>

#pragma once

char *ut_cstring_new_printf(const char *format, ...)
    __attribute((format(printf, 1, 2)));

char *ut_cstring_new_vprintf(const char *format, va_list ap);

static inline void ut_cstring_clear(char **string) {
  if (*string != NULL) {
    free(*string);
    *string = NULL;
  }
}

static inline char *ut_cstring_take(char **string) {
  char *result = *string;
  *string = NULL;
  return result;
}

#define ut_cstring_ref char *__attribute__((__cleanup__(ut_cstring_clear)))
