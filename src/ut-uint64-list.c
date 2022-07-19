#include <assert.h>

#include "ut-list.h"
#include "ut-uint64-array.h"
#include "ut-uint64-list.h"

int ut_uint64_list_id = 0;

UtObject *ut_uint64_list_new() { return ut_uint64_array_new(); }

UtObject *ut_uint64_list_new_from_elements(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_uint64_array_new_from_va_elements(length, ap);
  va_end(ap);
}

uint64_t ut_uint64_list_get_element(UtObject *object, size_t index) {
  UtUint64ListInterface *uint64_list_interface =
      ut_object_get_interface(object, &ut_uint64_list_id);
  assert(uint64_list_interface != NULL);
  return uint64_list_interface->get_element(object, index);
}

uint64_t *ut_uint64_list_take_data(UtObject *object) {
  UtUint64ListInterface *uint64_list_interface =
      ut_object_get_interface(object, &ut_uint64_list_id);
  assert(uint64_list_interface != NULL);
  return uint64_list_interface->take_data(object);
}

void ut_uint64_list_append(UtObject *object, uint64_t item) {
  ut_uint64_list_append_block(object, &item, 1);
}

void ut_uint64_list_append_block(UtObject *object, const uint64_t *data,
                                 size_t data_length) {
  size_t length = ut_list_get_length(object);
  ut_uint64_list_insert_block(object, length, data, data_length);
}

void ut_uint64_list_prepend(UtObject *object, uint64_t item) {
  ut_uint64_list_prepend_block(object, &item, 1);
}

void ut_uint64_list_prepend_block(UtObject *object, const uint64_t *data,
                                  size_t data_length) {
  ut_uint64_list_insert_block(object, 0, data, data_length);
}

void ut_uint64_list_insert(UtObject *object, size_t index, uint64_t item) {
  ut_uint64_list_insert_block(object, index, &item, 1);
}

void ut_uint64_list_insert_block(UtObject *object, size_t index,
                                 const uint64_t *data, size_t data_length) {
  UtUint64ListInterface *uint64_list_interface =
      ut_object_get_interface(object, &ut_uint64_list_id);
  assert(uint64_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  uint64_list_interface->insert(object, index, data, data_length);
}

bool ut_object_implements_uint64_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_uint64_list_id) != NULL;
}
