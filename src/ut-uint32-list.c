#include <assert.h>

#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-uint32-array.h"
#include "ut-uint32-list.h"

int ut_uint32_list_id = 0;

UtObject *ut_uint32_list_new() { return ut_uint32_array_new(); }

UtObject *ut_uint32_list_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_uint32_array_new_with_va_data(length, ap);
  va_end(ap);
}

uint32_t ut_uint32_list_get_element(UtObject *object, size_t index) {
  UtUint32ListInterface *uint32_list_interface =
      ut_object_get_interface(object, &ut_uint32_list_id);
  assert(uint32_list_interface != NULL);
  return uint32_list_interface->get_element(object, index);
}

uint32_t *ut_uint32_list_take_data(UtObject *object) {
  UtUint32ListInterface *uint32_list_interface =
      ut_object_get_interface(object, &ut_uint32_list_id);
  assert(uint32_list_interface != NULL);
  return uint32_list_interface->take_data(object);
}

void ut_uint32_list_append(UtObject *object, uint32_t item) {
  size_t length = ut_list_get_length(object);
  ut_uint32_list_insert(object, length, item);
}

void ut_uint32_list_prepend(UtObject *object, uint32_t item) {
  ut_uint32_list_insert(object, 0, item);
}

void ut_uint32_list_insert(UtObject *object, size_t index, uint32_t item) {
  UtUint32ListInterface *uint32_list_interface =
      ut_object_get_interface(object, &ut_uint32_list_id);
  assert(uint32_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  uint32_list_interface->insert(object, index, item);
}

bool ut_object_implements_uint32_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_uint32_list_id) != NULL;
}
