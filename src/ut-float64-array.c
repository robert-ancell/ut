#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut-float64-array.h"
#include "ut-float64-list.h"
#include "ut-float64.h"
#include "ut-list.h"
#include "ut-string.h"

typedef struct {
  UtObject object;
  double *data;
  size_t data_length;
} UtFloat64Array;

static void resize_list(UtFloat64Array *self, size_t length) {
  self->data = realloc(self->data, sizeof(double) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = 0;
  }
  self->data_length = length;
}

static double ut_float64_array_get_element(UtObject *object, size_t index) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  return self->data[index];
}

static double *ut_float64_array_take_data(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  double *result = self->data;
  self->data = NULL;
  self->data_length = 0;
  return result;
}

static void ut_float64_array_insert(UtObject *object, size_t index,
                                    const double *data, size_t data_length) {
  UtFloat64Array *self = (UtFloat64Array *)object;

  size_t orig_data_length = self->data_length;
  resize_list(self, self->data_length + data_length);

  // Shift existing data up
  for (size_t i = index; i < orig_data_length; i++) {
    size_t new_index = self->data_length - i - 1;
    size_t old_index = new_index - data_length;
    self->data[new_index] = self->data[old_index];
  }

  // Insert new data
  for (size_t i = 0; i < data_length; i++) {
    self->data[index + i] = data[i];
  }
}

static void ut_float64_array_insert_object(UtObject *object, size_t index,
                                           UtObject *item) {
  assert(ut_object_is_float64(item));
  double value = ut_float64_get_value(item);
  ut_float64_array_insert(object, index, &value, 1);
}

static void ut_float64_array_remove(UtObject *object, size_t index,
                                    size_t count) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize_list(self, self->data_length - count);
}

static void ut_float64_array_resize(UtObject *object, size_t length) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  resize_list(self, length);
}

static size_t ut_float64_array_get_length(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  return self->data_length;
}

static UtObject *ut_float64_array_get_element_object(UtObject *object,
                                                     size_t index) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  return ut_float64_new(self->data[index]);
}

static UtObject *ut_float64_array_copy(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  UtObject *copy = ut_float64_array_new();
  ut_float64_array_insert(copy, 0, self->data, self->data_length);
  return copy;
}

static void ut_float64_array_init(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_float64_array_to_string(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  UtObjectRef string = ut_string_new("<float64>[");
  for (size_t i = 0; i < self->data_length; i++) {
    if (i != 0) {
      ut_string_append(string, ", ");
    }
    char value_string[1024];
    snprintf(value_string, 1024, "%g", self->data[i]);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "]");

  return ut_string_take_text(string);
}

static void ut_float64_array_cleanup(UtObject *object) {
  UtFloat64Array *self = (UtFloat64Array *)object;
  free(self->data);
}

static UtFloat64ListInterface float64_list_interface = {
    .get_element = ut_float64_array_get_element,
    .take_data = ut_float64_array_take_data,
    .insert = ut_float64_array_insert};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_float64_array_get_length,
    .get_element = ut_float64_array_get_element_object,
    .copy = ut_float64_array_copy,
    .insert = ut_float64_array_insert_object,
    .remove = ut_float64_array_remove,
    .resize = ut_float64_array_resize};

static UtObjectInterface object_interface = {
    .type_name = "UtFloat64Array",
    .init = ut_float64_array_init,
    .to_string = ut_float64_array_to_string,
    .cleanup = ut_float64_array_cleanup,
    .interfaces = {{&ut_float64_list_id, &float64_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_float64_array_new() {
  return ut_object_new(sizeof(UtFloat64Array), &object_interface);
}

UtObject *ut_float64_array_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  UtObject *object = ut_float64_array_new_with_va_data(length, ap);
  va_end(ap);
  return object;
}

UtObject *ut_float64_array_new_with_va_data(size_t length, va_list ap) {
  UtObject *object = ut_float64_array_new();
  UtFloat64Array *self = (UtFloat64Array *)object;

  resize_list(self, length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = va_arg(ap, double);
  }

  return object;
}

double *ut_float64_array_get_data(UtObject *object) {
  assert(ut_object_is_float64_array(object));
  UtFloat64Array *self = (UtFloat64Array *)object;
  return self->data;
}

bool ut_object_is_float64_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
