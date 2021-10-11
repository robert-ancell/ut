#include <assert.h>
#include <stdlib.h>

#include "ut-list.h"
#include "ut-mutable-uint8-list.h"
#include "ut-object-private.h"
#include "ut-uint8-list.h"

typedef struct {
  uint8_t *data;
  size_t data_length;
} UtMutableUint8List;

const uint8_t *ut_mutable_uint8_list_get_data(UtObject *object) {
  UtMutableUint8List *self = ut_object_get_data(object);
  return self->data;
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_mutable_uint8_list_get_data};

size_t ut_mutable_uint8_list_get_length(UtObject *object) {
  UtMutableUint8List *self = ut_object_get_data(object);
  return self->data_length;
}

static UtListFunctions list_functions = {.get_length =
                                             ut_mutable_uint8_list_get_length};

static const char *ut_mutable_uint8_list_get_type_name() {
  return "MutableUint8List";
}

static void ut_mutable_uint8_list_init(UtObject *object) {
  UtMutableUint8List *self = ut_object_get_data(object);
  self->data = NULL;
  self->data_length = 0;
}

static void ut_mutable_uint8_list_cleanup(UtObject *object) {
  UtMutableUint8List *self = ut_object_get_data(object);
  free(self->data);
}

static UtObjectFunctions object_functions = {
    .get_type_name = ut_mutable_uint8_list_get_type_name,
    .init = ut_mutable_uint8_list_init,
    .cleanup = ut_mutable_uint8_list_cleanup,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions}}};

UtObject *ut_mutable_uint8_list_new() {
  return ut_object_new(sizeof(UtMutableUint8List), &object_functions);
}

void ut_mutable_uint8_list_append(UtObject *object, uint8_t data) {
  ut_mutable_uint8_list_append_block(object, &data, 1);
}

void ut_mutable_uint8_list_append_block(UtObject *object, const uint8_t *data,
                                        size_t data_length) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableUint8List *self = ut_object_get_data(object);
  ut_mutable_uint8_list_insert_block(object, self->data_length, data,
                                     data_length);
}

void ut_mutable_uint8_list_insert(UtObject *object, size_t index,
                                  uint8_t data) {
  ut_mutable_uint8_list_insert_block(object, index, &data, 1);
}

void ut_mutable_uint8_list_insert_block(UtObject *object, size_t index,
                                        const uint8_t *data,
                                        size_t data_length) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableUint8List *self = ut_object_get_data(object);

  size_t orig_data_length = self->data_length;
  self->data_length += data_length;
  self->data = realloc(self->data, sizeof(uint8_t) * self->data_length);

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

bool ut_object_is_mutable_uint8_list(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
