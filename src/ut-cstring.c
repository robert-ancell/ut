#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-cstring.h"

char *ut_cstring_new(const char *value) { return strdup(value); }

char *ut_cstring_new_printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  char *result = ut_cstring_new_vprintf(format, ap);
  va_end(ap);

  return result;
}

char *ut_cstring_new_vprintf(const char *format, va_list ap) {
  char empty = '\0';
  va_list ap2;
  va_copy(ap2, ap);
  int length = vsnprintf(&empty, 1, format, ap2);
  va_end(ap2);
  char *result = malloc(sizeof(char) * (length + 1));
  vsnprintf(result, length + 1, format, ap);
  return result;
}

void ut_cstring_set(char **string, const char *value) {
  if (*string != NULL) {
    free(*string);
  }
  *string = value != NULL ? strdup(value) : NULL;
}

void ut_cstring_clear(char **string) {
  if (*string != NULL) {
    free(*string);
    *string = NULL;
  }
}

char *ut_cstring_take(char **string) {
  char *result = *string;
  *string = NULL;
  return result;
}

bool ut_cstring_starts_with(const char *value, const char *prefix) {
  assert(value != NULL);
  assert(prefix != NULL);

  size_t i = 0;
  while (value[i] != '\0' && prefix[i] != '\0' && value[i] == prefix[i]) {
    i++;
  }
  return prefix[i] == '\0';
}

bool ut_cstring_ends_with(const char *value, const char *suffix) {
  assert(value != NULL);
  assert(suffix != NULL);

  size_t value_length = strlen(value);
  size_t suffix_length = strlen(suffix);
  if (suffix_length > value_length) {
    return false;
  }
  return ut_cstring_starts_with(value + value_length - suffix_length, suffix);
}

char *ut_cstring_join(const char *separator, const char *value0, ...) {
  if (separator == NULL) {
    separator = "";
  }
  if (value0 == NULL) {
    return strdup("");
  }

  size_t separator_length = strlen(separator);
  size_t value0_length = strlen(value0);

  size_t value_length = 1;
  size_t result_length = value0_length;
  va_list ap;

  va_list ap2;
  va_copy(ap2, ap);
  va_start(ap2, value0);
  while (true) {
    const char *v = va_arg(ap2, const char *);
    if (v == NULL) {
      break;
    }
    value_length++;
    result_length += strlen(v);
  }
  va_end(ap2);

  result_length += (value_length - 1) * strlen(separator);
  char *result = malloc(sizeof(char) * (result_length + 1));
  memcpy(result, value0, value0_length);
  size_t offset = value0_length;
  va_start(ap, value0);
  for (size_t i = 1; i < value_length; i++) {
    memcpy(result + offset, separator, separator_length);
    offset += separator_length;
    const char *v = va_arg(ap, const char *);
    size_t v_length = strlen(v);
    memcpy(result + offset, v, v_length);
    offset += v_length;
  }
  va_end(ap);
  result[offset] = '\0';

  return result;
}

char *ut_cstring_substring(const char *value, size_t start, size_t end) {
  assert(end > start);
  return strndup(value + start, end - start);
}
