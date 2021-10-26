#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

int ut_uint8_list_id = 0;

UtObject *ut_uint8_list_new() { return ut_uint8_array_new(); }

UtObject *ut_uint8_list_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_uint8_array_new_with_va_data(length, ap);
  va_end(ap);
}

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->get_element(object, index);
}

uint8_t *ut_uint8_list_take_data(UtObject *object) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->take_data(object);
}

void ut_uint8_list_append(UtObject *object, uint8_t item) {
  size_t length = ut_list_get_length(object);
  ut_uint8_list_insert(object, length, item);
}

void ut_uint8_list_prepend(UtObject *object, uint8_t item) {
  ut_uint8_list_insert(object, 0, item);
}

void ut_uint8_list_insert(UtObject *object, size_t index, uint8_t item) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  uint8_list_interface->insert(object, index, item);
}

bool ut_object_implements_uint8_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_uint8_list_id) != NULL;
}
