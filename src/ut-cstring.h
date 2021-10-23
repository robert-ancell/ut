#include <stdlib.h>

#pragma once

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

#define ut_cstring char *__attribute__((__cleanup__(ut_cstring_clear)))
