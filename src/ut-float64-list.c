#include <assert.h>

#include "ut-float64-array.h"
#include "ut-float64-list.h"
#include "ut-list.h"
#include "ut-object-private.h"

int ut_float64_list_id = 0;

UtObject *ut_float64_list_new() { return ut_float64_array_new(); }

UtObject *ut_float64_list_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_float64_array_new_with_va_data(length, ap);
  va_end(ap);
}

double ut_float64_list_get_element(UtObject *object, size_t index) {
  UtFloat64ListInterface *float64_list_interface =
      ut_object_get_interface(object, &ut_float64_list_id);
  assert(float64_list_interface != NULL);
  return float64_list_interface->get_element(object, index);
}

double *ut_float64_list_take_data(UtObject *object) {
  UtFloat64ListInterface *float64_list_interface =
      ut_object_get_interface(object, &ut_float64_list_id);
  assert(float64_list_interface != NULL);
  return float64_list_interface->take_data(object);
}

void ut_float64_list_append(UtObject *object, double item) {
  size_t length = ut_list_get_length(object);
  ut_float64_list_insert(object, length, item);
}

void ut_float64_list_prepend(UtObject *object, double item) {
  ut_float64_list_insert(object, 0, item);
}

void ut_float64_list_insert(UtObject *object, size_t index, double item) {
  UtFloat64ListInterface *float64_list_interface =
      ut_object_get_interface(object, &ut_float64_list_id);
  assert(float64_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  float64_list_interface->insert(object, index, item);
}

bool ut_object_implements_float64_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_float64_list_id) != NULL;
}
