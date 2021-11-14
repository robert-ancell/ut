#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-string.h"
#include "ut-uint16-array.h"
#include "ut-uint16-list.h"
#include "ut-uint16.h"

typedef struct {
  UtObject object;
  uint16_t *data;
  size_t data_length;
} UtUint16Array;

static void resize_list(UtUint16Array *self, size_t length) {
  self->data = realloc(self->data, sizeof(uint16_t) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = 0;
  }
  self->data_length = length;
}

static uint16_t ut_uint16_array_get_element(UtObject *object, size_t index) {
  UtUint16Array *self = (UtUint16Array *)object;
  return self->data[index];
}

static uint16_t *ut_uint16_array_take_data(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  uint16_t *result = self->data;
  self->data = NULL;
  self->data_length = 0;
  return result;
}

static void ut_uint16_array_insert(UtObject *object, size_t index,
                                   const uint16_t *data, size_t data_length) {
  assert(ut_object_is_uint16_array(object));
  UtUint16Array *self = (UtUint16Array *)object;

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

static void ut_uint16_array_insert_object(UtObject *object, size_t index,
                                          UtObject *item) {
  assert(ut_object_is_uint16(item));
  uint16_t value = ut_uint16_get_value(item);
  ut_uint16_array_insert(object, index, &value, 1);
}

static void ut_uint16_array_remove(UtObject *object, size_t index,
                                   size_t count) {
  UtUint16Array *self = (UtUint16Array *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize_list(self, self->data_length - count);
}

static void ut_uint16_array_resize(UtObject *object, size_t length) {
  UtUint16Array *self = (UtUint16Array *)object;
  resize_list(self, length);
}

static size_t ut_uint16_array_get_length(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  return self->data_length;
}

static UtObject *ut_uint16_array_get_element_object(UtObject *object,
                                                    size_t index) {
  UtUint16Array *self = (UtUint16Array *)object;
  return ut_uint16_new(self->data[index]);
}

static UtObject *ut_uint16_array_copy(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  UtObject *copy = ut_uint16_array_new();
  ut_uint16_array_insert(copy, 0, self->data, self->data_length);
  return copy;
}

static void ut_uint16_array_read(UtObject *object,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUint16Array *self = (UtUint16Array *)object;
  size_t n_used = callback(user_data, object);
  UtObjectRef unused_data = NULL;
  if (n_used != self->data_length) {
    unused_data = ut_uint16_array_new();
    ut_uint16_array_insert(unused_data, 0, self->data + n_used,
                           self->data_length - n_used);
  }
  UtObjectRef eos = ut_end_of_stream_new(unused_data);
  callback(user_data, eos);
}

static void ut_uint16_array_init(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_uint16_array_to_string(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  UtObjectRef string = ut_string_new("<uint16>[");
  for (size_t i = 0; i < self->data_length; i++) {
    if (i != 0) {
      ut_string_append(string, ", ");
    }
    char value_string[6];
    snprintf(value_string, 6, "%d", self->data[i]);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "]");

  return ut_string_take_text(string);
}

static void ut_uint16_array_cleanup(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  free(self->data);
}

static UtUint16ListInterface uint16_list_interface = {
    .get_element = ut_uint16_array_get_element,
    .take_data = ut_uint16_array_take_data,
    .insert = ut_uint16_array_insert};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_uint16_array_get_length,
    .get_element = ut_uint16_array_get_element_object,
    .copy = ut_uint16_array_copy,
    .insert = ut_uint16_array_insert_object,
    .remove = ut_uint16_array_remove,
    .resize = ut_uint16_array_resize};

static UtInputStreamInterface input_stream_interface = {
    .read = ut_uint16_array_read, .read_all = ut_uint16_array_read};

static UtObjectInterface object_interface = {
    .type_name = "UtUint16Array",
    .init = ut_uint16_array_init,
    .to_string = ut_uint16_array_to_string,
    .cleanup = ut_uint16_array_cleanup,
    .interfaces = {{&ut_uint16_list_id, &uint16_list_interface},
                   {&ut_list_id, &list_interface},
                   {&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_uint16_array_new() {
  return ut_object_new(sizeof(UtUint16Array), &object_interface);
}

UtObject *ut_uint16_array_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  UtObject *object = ut_uint16_array_new_with_va_data(length, ap);
  va_end(ap);
  return object;
}

UtObject *ut_uint16_array_new_with_va_data(size_t length, va_list ap) {
  UtObject *object = ut_uint16_array_new();
  UtUint16Array *self = (UtUint16Array *)object;

  resize_list(self, length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = va_arg(ap, int);
  }

  return object;
}

uint16_t *ut_uint16_array_get_data(UtObject *object) {
  assert(ut_object_is_uint16_array(object));
  UtUint16Array *self = (UtUint16Array *)object;
  return self->data;
}

bool ut_object_is_uint16_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
