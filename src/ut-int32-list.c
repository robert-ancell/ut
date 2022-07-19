#include <assert.h>

#include "ut-int32-array.h"
#include "ut-int32-list.h"
#include "ut-list.h"

int ut_int32_list_id = 0;

UtObject *ut_int32_list_new() { return ut_int32_array_new(); }

UtObject *ut_int32_list_new_from_elements(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_int32_array_new_from_va_elements(length, ap);
  va_end(ap);
}

int32_t ut_int32_list_get_element(UtObject *object, size_t index) {
  UtInt32ListInterface *int32_list_interface =
      ut_object_get_interface(object, &ut_int32_list_id);
  assert(int32_list_interface != NULL);
  return int32_list_interface->get_element(object, index);
}

int32_t *ut_int32_list_take_data(UtObject *object) {
  UtInt32ListInterface *int32_list_interface =
      ut_object_get_interface(object, &ut_int32_list_id);
  assert(int32_list_interface != NULL);
  return int32_list_interface->take_data(object);
}

void ut_int32_list_append(UtObject *object, int32_t item) {
  ut_int32_list_append_block(object, &item, 1);
}

void ut_int32_list_append_block(UtObject *object, const int32_t *data,
                                size_t data_length) {
  size_t length = ut_list_get_length(object);
  ut_int32_list_insert_block(object, length, data, data_length);
}

void ut_int32_list_prepend(UtObject *object, int32_t item) {
  ut_int32_list_prepend_block(object, &item, 1);
}

void ut_int32_list_prepend_block(UtObject *object, const int32_t *data,
                                 size_t data_length) {
  ut_int32_list_insert_block(object, 0, data, data_length);
}

void ut_int32_list_insert(UtObject *object, size_t index, int32_t item) {
  ut_int32_list_insert_block(object, index, &item, 1);
}

void ut_int32_list_insert_block(UtObject *object, size_t index,
                                const int32_t *data, size_t data_length) {
  UtInt32ListInterface *int32_list_interface =
      ut_object_get_interface(object, &ut_int32_list_id);
  assert(int32_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  int32_list_interface->insert(object, index, data, data_length);
}

bool ut_object_implements_int32_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_int32_list_id) != NULL;
}
