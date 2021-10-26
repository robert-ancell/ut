#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
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

static void ut_uint8_array_insert_object(UtObject *object, size_t index,
                                         UtObject *item) {
  assert(ut_object_is_uint8(item));
  ut_uint8_array_insert(object, index, ut_uint8_get_value(item));
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

static UtObject *ut_uint8_array_copy(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_array_append_block(copy, self->data, self->data_length);
  return copy;
}

static void ut_uint8_array_read(UtObject *object,
                                UtInputStreamCallback callback, void *user_data,
                                UtObject *cancel) {
  UtUint8Array *self = (UtUint8Array *)object;
  size_t n_used = callback(user_data, object);
  UtObjectRef unused_data = NULL;
  if (n_used != self->data_length) {
    unused_data = ut_uint8_array_new();
    ut_uint8_array_append_block(unused_data, self->data + n_used,
                                self->data_length - n_used);
  }
  UtObjectRef eos = ut_end_of_stream_new(unused_data);
  callback(user_data, eos);
}

static void ut_uint8_array_init(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static void ut_uint8_array_cleanup(UtObject *object) {
  UtUint8Array *self = (UtUint8Array *)object;
  free(self->data);
}

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_uint8_array_get_element,
    .take_data = ut_uint8_array_take_data,
    .insert = ut_uint8_array_insert};

static UtListInterface list_interface = {
    .is_mutable = true,
    .get_length = ut_uint8_array_get_length,
    .get_element = ut_uint8_array_get_element_object,
    .copy = ut_uint8_array_copy,
    .insert = ut_uint8_array_insert_object,
    .remove = ut_uint8_array_remove,
    .resize = ut_uint8_array_resize};

static UtInputStreamInterface input_stream_interface = {
    .read = ut_uint8_array_read, .read_all = ut_uint8_array_read};

static UtObjectInterface object_interface = {
    .type_name = "UtUint8Array",
    .init = ut_uint8_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_uint8_array_cleanup,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_uint8_array_new() {
  return ut_object_new(sizeof(UtUint8Array), &object_interface);
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

UtObject *ut_uint8_array_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  UtObject *object = ut_uint8_array_new_with_va_data(length, ap);
  va_end(ap);
  return object;
}

void ut_uint8_array_append(UtObject *object, uint8_t data) {
  ut_uint8_array_append_block(object, &data, 1);
}

void ut_uint8_array_append_block(UtObject *object, const uint8_t *data,
                                 size_t data_length) {
  assert(ut_object_is_uint8_array(object));
  UtUint8Array *self = (UtUint8Array *)object;
  ut_uint8_array_insert_block(object, self->data_length, data, data_length);
}

void ut_uint8_array_insert(UtObject *object, size_t index, uint8_t data) {
  ut_uint8_array_insert_block(object, index, &data, 1);
}

void ut_uint8_array_insert_block(UtObject *object, size_t index,
                                 const uint8_t *data, size_t data_length) {
  assert(ut_object_is_uint8_array(object));
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

uint8_t *ut_uint8_array_get_data(UtObject *object) {
  assert(ut_object_is_uint8_array(object));
  UtUint8Array *self = (UtUint8Array *)object;
  return self->data;
}

bool ut_object_is_uint8_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
