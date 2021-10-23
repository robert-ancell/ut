#include <assert.h>
#include <stdlib.h>

#include "ut-end-of-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-object-private.h"
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

static const uint16_t *ut_uint16_array_get_list_data(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  return self->data;
}

static void ut_uint16_array_insert_object(UtObject *object, size_t index,
                                          UtObject *item) {
  assert(ut_object_is_uint16(item));
  ut_uint16_array_insert(object, index, ut_uint16_get_value(item));
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

static UtObject *ut_uint16_array_get_element(UtObject *object, size_t index) {
  UtUint16Array *self = (UtUint16Array *)object;
  return ut_uint16_new(self->data[index]);
}

static void ut_uint16_array_read(UtObject *object, size_t block_size,
                                 UtInputStreamCallback callback,
                                 void *user_data, UtObject *cancel) {
  UtUint16Array *self = (UtUint16Array *)object;
  size_t n_used = callback(user_data, object);
  UtObjectRef unused_data = NULL;
  if (n_used != self->data_length) {
    unused_data = ut_uint16_array_new();
    ut_uint16_array_append_block(unused_data, self->data + n_used,
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

static void ut_uint16_array_cleanup(UtObject *object) {
  UtUint16Array *self = (UtUint16Array *)object;
  free(self->data);
}

static UtUint16ListFunctions uint16_list_functions = {
    .get_data = ut_uint16_array_get_list_data};

static UtMutableListFunctions mutable_list_functions = {
    .insert = ut_uint16_array_insert_object,
    .remove = ut_uint16_array_remove,
    .resize = ut_uint16_array_resize};

static UtListFunctions list_functions = {
    .get_length = ut_uint16_array_get_length,
    .get_element = ut_uint16_array_get_element};

static UtInputStreamFunctions input_stream_functions = {
    .read = ut_uint16_array_read, .read_all = ut_uint16_array_read};

static UtObjectFunctions object_functions = {
    .type_name = "UtUint16Array",
    .init = ut_uint16_array_init,
    .to_string = ut_list_to_string,
    .cleanup = ut_uint16_array_cleanup,
    .interfaces = {{&ut_uint16_list_id, &uint16_list_functions},
                   {&ut_mutable_list_id, &mutable_list_functions},
                   {&ut_list_id, &list_functions},
                   {&ut_input_stream_id, &input_stream_functions},
                   {NULL, NULL}}};

UtObject *ut_uint16_array_new() {
  return ut_object_new(sizeof(UtUint16Array), &object_functions);
}

void ut_uint16_array_append(UtObject *object, uint16_t data) {
  ut_uint16_array_append_block(object, &data, 1);
}

void ut_uint16_array_append_block(UtObject *object, const uint16_t *data,
                                  size_t data_length) {
  assert(ut_object_is_uint16_array(object));
  UtUint16Array *self = (UtUint16Array *)object;
  ut_uint16_array_insert_block(object, self->data_length, data, data_length);
}

void ut_uint16_array_insert(UtObject *object, size_t index, uint16_t data) {
  ut_uint16_array_insert_block(object, index, &data, 1);
}

void ut_uint16_array_insert_block(UtObject *object, size_t index,
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

uint16_t *ut_uint16_array_get_data(UtObject *object) {
  assert(ut_object_is_uint16_array(object));
  UtUint16Array *self = (UtUint16Array *)object;
  return self->data;
}

bool ut_object_is_uint16_array(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
