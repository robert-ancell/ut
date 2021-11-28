#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-list.h"
#include "ut-output-stream.h"
#include "ut-string.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-uint8-subarray.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  uint8_t *data;
  size_t data_length;
} UtUint8Array;

static void resize_list(UtUint8Array *self, size_t length) {
  self->data = realloc(self->data, sizeof(uint8_t) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = 0;
  }
  self->data_length = length;
}

static uint8_t ut_uint8_array_get_element(UtObject *object, size_t index) {
  UtUint8Array *self = (UtUint8Array *)object;
  return self->data[index];
}

static uint8_t *ut_uint8_array_take_data(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  uint8_t *result = self->data;
  self->data = NULL;
  self->data_length = 0;
  return result;
}

static void ut_uint8_array_insert(UtObject *object, size_t index,
                                  const uint8_t *data, size_t data_length) {
  UtUint8Array *self = (UtUint8Array *)object;

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

static void ut_uint8_array_append(UtObject *object, const uint8_t *data,
                                  size_t data_length) {
  UtUint8Array *self = (UtUint8Array *)object;
  ut_uint8_array_insert(object, self->data_length, data, data_length);
}

static void ut_uint8_array_insert_object(UtObject *object, size_t index,
                                         UtObject *item) {
  assert(ut_object_is_uint8(item));
  uint8_t value = ut_uint8_get_value(item);
  ut_uint8_array_insert(object, index, &value, 1);
}

static void ut_uint8_array_remove(UtObject *object, size_t index,
                                  size_t count) {
  UtUint8Array *self = (UtUint8Array *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize_list(self, self->data_length - count);
}

static void ut_uint8_array_resize(UtObject *object, size_t length) {
  UtUint8Array *self = (UtUint8Array *)object;
  resize_list(self, length);
}

static size_t ut_uint8_array_get_length(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  return self->data_length;
}

static UtObject *ut_uint8_array_get_element_object(UtObject *object,
                                                   size_t index) {
  UtUint8Array *self = (UtUint8Array *)object;
  return ut_uint8_new(self->data[index]);
}

static UtObject *ut_uint8_array_get_sublist(UtObject *object, size_t start,
                                            size_t count) {
  return ut_uint8_subarray_new(object, start, count);
}

static UtObject *ut_uint8_array_copy(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_array_append(copy, self->data, self->data_length);
  return copy;
}

static void ut_uint8_array_write(UtObject *object, UtObject *data,
                                 UtOutputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUint8Array *self = (UtUint8Array *)object;

  if (ut_object_is_uint8_array(data)) {
    ut_uint8_array_append(object, ut_uint8_array_get_data(data),
                          ut_uint8_array_get_length(data));
  } else {
    size_t data_length = ut_list_get_length(data);
    size_t start = self->data_length;
    resize_list(self, self->data_length + data_length);
    for (size_t i = 0; i < data_length; i++) {
      self->data[start + i] = ut_uint8_list_get_element(data, i);
    }
  }

  if (callback != NULL) {
    callback(user_data, NULL);
  }
}

static void ut_uint8_array_init(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_uint8_array_to_string(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  UtObjectRef string = ut_string_new("<uint8>[");
  for (size_t i = 0; i < self->data_length; i++) {
    if (i != 0) {
      ut_string_append(string, ", ");
    }
    char value_string[4];
    snprintf(value_string, 4, "%d", self->data[i]);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "]");

  return ut_string_take_text(string);
}

static void ut_uint8_array_cleanup(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  free(self->data);
}

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_uint8_array_get_element,
    .take_data = ut_uint8_array_take_data,
    .insert = ut_uint8_array_insert,
    .append = ut_uint8_array_append};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_uint8_array_get_length,
    .get_element = ut_uint8_array_get_element_object,
    .get_sublist = ut_uint8_array_get_sublist,
    .copy = ut_uint8_array_copy,
    .insert = ut_uint8_array_insert_object,
    .remove = ut_uint8_array_remove,
    .resize = ut_uint8_array_resize};

static UtOutputStreamInterface output_stream_interface = {
    .write = ut_uint8_array_write};

static UtObjectInterface object_interface = {
    .type_name = "UtUint8Array",
    .init = ut_uint8_array_init,
    .to_string = ut_uint8_array_to_string,
    .cleanup = ut_uint8_array_cleanup,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {&ut_output_stream_id, &output_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_uint8_array_new() {
  return ut_object_new(sizeof(UtUint8Array), &object_interface);
}

UtObject *ut_uint8_array_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  UtObject *object = ut_uint8_array_new_with_va_data(length, ap);
  va_end(ap);
  return object;
}

UtObject *ut_uint8_array_new_with_va_data(size_t length, va_list ap) {
  UtObject *object = ut_uint8_array_new();
  UtUint8Array *self = (UtUint8Array *)object;

  resize_list(self, length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = va_arg(ap, int);
  }

  return object;
}

uint8_t *ut_uint8_array_get_data(UtObject *object) {
  assert(ut_object_is_uint8_array(object));
  UtUint8Array *self = (UtUint8Array *)object;
  return self->data;
}

bool ut_object_is_uint8_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
