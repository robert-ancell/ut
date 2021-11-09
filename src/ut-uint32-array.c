#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-string.h"
#include "ut-uint32-array.h"
#include "ut-uint32-list.h"
#include "ut-uint32.h"

typedef struct {
  UtObject object;
  uint32_t *data;
  size_t data_length;
} UtUint32Array;

static void resize_list(UtUint32Array *self, size_t length) {
  self->data = realloc(self->data, sizeof(uint32_t) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = 0;
  }
  self->data_length = length;
}

static uint32_t ut_uint32_array_get_element(UtObject *object, size_t index) {
  UtUint32Array *self = (UtUint32Array *)object;
  return self->data[index];
}

static uint32_t *ut_uint32_array_take_data(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  uint32_t *result = self->data;
  self->data = NULL;
  self->data_length = 0;
  return result;
}

static void ut_uint32_array_insert_object(UtObject *object, size_t index,
                                          UtObject *item) {
  assert(ut_object_is_uint32(item));
  ut_uint32_array_insert(object, index, ut_uint32_get_value(item));
}

static void ut_uint32_array_remove(UtObject *object, size_t index,
                                   size_t count) {
  UtUint32Array *self = (UtUint32Array *)object;
  assert(index <= self->data_length);
  assert(index + count <= self->data_length);
  for (size_t i = index; i < self->data_length - count; i++) {
    self->data[i] = self->data[i + count];
  }
  resize_list(self, self->data_length - count);
}

static void ut_uint32_array_resize(UtObject *object, size_t length) {
  UtUint32Array *self = (UtUint32Array *)object;
  resize_list(self, length);
}

static size_t ut_uint32_array_get_length(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  return self->data_length;
}

static UtObject *ut_uint32_array_get_element_object(UtObject *object,
                                                    size_t index) {
  UtUint32Array *self = (UtUint32Array *)object;
  return ut_uint32_new(self->data[index]);
}

static UtObject *ut_uint32_array_copy(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  UtObject *copy = ut_uint32_array_new();
  ut_uint32_array_append_block(copy, self->data, self->data_length);
  return copy;
}

static void ut_uint32_array_read(UtObject *object,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUint32Array *self = (UtUint32Array *)object;
  size_t n_used = callback(user_data, object);
  UtObjectRef unused_data = NULL;
  if (n_used != self->data_length) {
    unused_data = ut_uint32_array_new();
    ut_uint32_array_append_block(unused_data, self->data + n_used,
                                 self->data_length - n_used);
  }
  UtObjectRef eos = ut_end_of_stream_new(unused_data);
  callback(user_data, eos);
}

static void ut_uint32_array_init(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_uint32_array_to_string(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  UtObjectRef string = ut_string_new("<uint32>[");
  for (size_t i = 0; i < self->data_length; i++) {
    if (i != 0) {
      ut_string_append(string, ", ");
    }
    char value_string[11];
    snprintf(value_string, 11, "%d", self->data[i]);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "]");

  return ut_string_take_text(string);
}

static void ut_uint32_array_cleanup(UtObject *object) {
  UtUint32Array *self = (UtUint32Array *)object;
  free(self->data);
}

static UtUint32ListInterface uint32_list_interface = {
    .get_element = ut_uint32_array_get_element,
    .take_data = ut_uint32_array_take_data,
    .insert = ut_uint32_array_insert};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_uint32_array_get_length,
    .get_element = ut_uint32_array_get_element_object,
    .copy = ut_uint32_array_copy,
    .insert = ut_uint32_array_insert_object,
    .remove = ut_uint32_array_remove,
    .resize = ut_uint32_array_resize};

static UtInputStreamInterface input_stream_interface = {
    .read = ut_uint32_array_read, .read_all = ut_uint32_array_read};

static UtObjectInterface object_interface = {
    .type_name = "UtUint32Array",
    .init = ut_uint32_array_init,
    .to_string = ut_uint32_array_to_string,
    .cleanup = ut_uint32_array_cleanup,
    .interfaces = {{&ut_uint32_list_id, &uint32_list_interface},
                   {&ut_list_id, &list_interface},
                   {&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_uint32_array_new() {
  return ut_object_new(sizeof(UtUint32Array), &object_interface);
}

UtObject *ut_uint32_array_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  UtObject *object = ut_uint32_array_new_with_va_data(length, ap);
  va_end(ap);
  return object;
}

UtObject *ut_uint32_array_new_with_va_data(size_t length, va_list ap) {
  UtObject *object = ut_uint32_array_new();
  UtUint32Array *self = (UtUint32Array *)object;

  resize_list(self, length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = va_arg(ap, int);
  }

  return object;
}

void ut_uint32_array_append(UtObject *object, uint32_t data) {
  ut_uint32_array_append_block(object, &data, 1);
}

void ut_uint32_array_append_block(UtObject *object, const uint32_t *data,
                                  size_t data_length) {
  assert(ut_object_is_uint32_array(object));
  UtUint32Array *self = (UtUint32Array *)object;
  ut_uint32_array_insert_block(object, self->data_length, data, data_length);
}

void ut_uint32_array_insert(UtObject *object, size_t index, uint32_t data) {
  ut_uint32_array_insert_block(object, index, &data, 1);
}

void ut_uint32_array_insert_block(UtObject *object, size_t index,
                                  const uint32_t *data, size_t data_length) {
  assert(ut_object_is_uint32_array(object));
  UtUint32Array *self = (UtUint32Array *)object;

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

uint32_t *ut_uint32_array_get_data(UtObject *object) {
  assert(ut_object_is_uint32_array(object));
  UtUint32Array *self = (UtUint32Array *)object;
  return self->data;
}

bool ut_object_is_uint32_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
