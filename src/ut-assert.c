#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut-assert.h"
#include "ut-cstring.h"
#include "ut-error.h"
#include "ut-list.h"
#include "ut-string.h"
#include "ut-uint8-list.h"

static char *append_string(char *value, const char *suffix) {
  size_t value_length = value != NULL ? strlen(value) : 0;
  size_t suffix_length = strlen(suffix);

  value = realloc(value, value_length + suffix_length + 1);
  memcpy(value + value_length, suffix, suffix_length + 1);

  return value;
}

static char *escape_character(char *value, char c) {
  size_t value_length = strlen(value);

  value_length++;
  value = realloc(value, value_length);
  if (c < 32 || c >= 127) {
    char text[5];
    snprintf(text, 5, "\\x%02x", c);
    value = append_string(value, text);
  }
  if (c == '\t') {
    value = append_string(value, "\\t");
  } else {
    char text[2] = {c, '\0'};
    value = append_string(value, text);
  }

  return value;
}

static char *escape_string(const char *value) {
  if (value == NULL) {
    return strdup("NULL");
  }
  char *escaped = append_string(NULL, "\"");
  for (const char *c = value; *c != '\0'; c++) {
    escaped = escape_character(escaped, *c);
  }
  return append_string(escaped, "\"");
}

void _ut_assert_equal(const char *file, int line, const char *a_name,
                      UtObject *a_value, const char *b_name,
                      UtObject *b_value) {
  if (a_value == NULL && b_value == NULL) {
    return;
  }
  if (a_value != NULL && b_value != NULL && ut_object_equal(a_value, b_value)) {
    return;
  }

  ut_cstring_ref a_value_string =
      a_value != NULL ? ut_object_to_string(a_value) : strdup("NULL");
  ut_cstring_ref b_value_string =
      b_value != NULL ? ut_object_to_string(b_value) : strdup("NULL");
  fprintf(stderr,
          "%s:%d Objects %s and %s are not equal:\n"
          "  %s\n"
          "  %s\n",
          file, line, a_name, b_name, a_value_string, b_value_string);

  abort();
}

void _ut_assert_is_error(const char *file, int line, const char *name,
                         UtObject *value) {
  if (ut_object_implements_error(value)) {
    return;
  }

  ut_cstring_ref value_string = ut_object_to_string(value);
  fprintf(stderr,
          "%s:%d Object %s is not an error:\n"
          "  %s\n",
          file, line, name, value_string);

  abort();
}

void _ut_assert_is_not_error(const char *file, int line, const char *name,
                             UtObject *value) {
  if (!ut_object_implements_error(value)) {
    return;
  }

  ut_cstring_ref value_string = ut_object_to_string(value);
  fprintf(stderr,
          "%s:%d Object %s is an error:\n"
          "  %s\n",
          file, line, name, value_string);

  abort();
}

void _ut_assert_cstring_equal(const char *file, int line, const char *a_name,
                              const char *a_value, const char *b_name,
                              const char *b_value) {
  if (a_value == NULL && b_value == NULL) {
    return;
  }
  if (a_value != NULL && b_value != NULL && strcmp(a_value, b_value) == 0) {
    return;
  }

  ut_cstring_ref escaped_a_value = escape_string(a_value);
  ut_cstring_ref escaped_b_value = escape_string(b_value);
  fprintf(stderr,
          "%s:%d Strings %s and %s are not equal:\n"
          "  %s\n"
          "  %s\n"
          "  ",
          file, line, a_name, b_name, escaped_a_value, escaped_b_value);
  for (size_t i = 0; escaped_a_value[i] != '\0' && escaped_b_value[i] != '\0' &&
                     escaped_a_value[i] == escaped_b_value[i];
       i++) {
    fprintf(stderr, " ");
  }
  fprintf(stderr, "^\n");

  abort();
}

void _ut_assert_uint8_list_equal(const char *file, int line, const char *a_name,
                                 UtObject *a_value, uint8_t *b_value,
                                 size_t b_length) {
  if (ut_list_get_length(a_value) == b_length) {
    bool match = true;
    for (size_t i = 0; i < b_length; i++) {
      if (ut_uint8_list_get_element(a_value, i) != b_value[i]) {
        match = false;
        break;
      }
    }

    if (match) {
      return;
    }
  }

  UtObjectRef b_value_string = ut_string_new("<uint8>[");
  for (size_t i = 0; i < b_length; i++) {
    if (i != 0) {
      ut_string_append(b_value_string, ", ");
    }
    ut_string_append_printf(b_value_string, "%d", b_value[i]);
  }
  ut_string_append(b_value_string, "]");

  ut_cstring_ref a_value_string = ut_object_to_string(a_value);
  fprintf(stderr,
          "%s:%d List %s doesn't have expected content:\n"
          "  %s\n"
          "  %s\n",
          file, line, a_name, a_value_string,
          ut_string_get_text(b_value_string));

  abort();
}
