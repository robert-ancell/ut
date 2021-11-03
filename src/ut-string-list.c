#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-string-array.h"
#include "ut-string-list.h"

int ut_string_list_id = 0;

UtObject *ut_string_list_new() { return ut_string_array_new(); }

UtObject *ut_string_list_new_with_data(const char *value, ...) {
  va_list ap;
  va_start(ap, value);
  return ut_string_array_new_with_va_data(value, ap);
  va_end(ap);
}

const char *ut_string_list_get_element(UtObject *object, size_t index) {
  UtStringListInterface *string_list_interface =
      ut_object_get_interface(object, &ut_string_list_id);
  assert(string_list_interface != NULL);
  return string_list_interface->get_element(object, index);
}

char **ut_string_list_take_data(UtObject *object) {
  UtStringListInterface *string_list_interface =
      ut_object_get_interface(object, &ut_string_list_id);
  assert(string_list_interface != NULL);
  return string_list_interface->take_data(object);
}

void ut_string_list_append(UtObject *object, const char *item) {
  size_t length = ut_list_get_length(object);
  ut_string_list_insert(object, length, item);
}

void ut_string_list_prepend(UtObject *object, const char *item) {
  ut_string_list_insert(object, 0, item);
}

void ut_string_list_insert(UtObject *object, size_t index, const char *item) {
  UtStringListInterface *string_list_interface =
      ut_object_get_interface(object, &ut_string_list_id);
  assert(string_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  string_list_interface->insert(object, index, item);
}

bool ut_object_implements_string_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_string_list_id) != NULL;
}