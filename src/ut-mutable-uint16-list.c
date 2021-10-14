#include <assert.h>
#include <stdlib.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-uint16-list.h"
#include "ut-object-private.h"
#include "ut-uint16-list.h"
#include "ut-uint16.h"

typedef struct {
  uint16_t *data;
  size_t data_length;
} UtMutableUint16List;

static void resize_list(UtMutableUint16List *self, size_t length) {
  self->data = realloc(self->data, sizeof(uint16_t) * length);
  for (size_t i = self->data_length; i < length; i++) {
    self->data[i] = 0;
  }
  self->data_length = length;
}

const uint16_t *ut_mutable_uint16_list_get_list_data(UtObject *object) {
  UtMutableUint16List *self = ut_object_get_data(object);
  return self->data;
}

static UtUint16ListFunctions uint16_list_functions = {
    .get_data = ut_mutable_uint16_list_get_list_data};

void ut_mutable_uint16_list_resize(UtObject *object, size_t length) {
  UtMutableUint16List *self = ut_object_get_data(object);
  resize_list(self, length);
}

static UtMutableListFunctions mutable_list_functions = {
    .resize = ut_mutable_uint16_list_resize};

static size_t ut_mutable_uint16_list_get_length(UtObject *object) {
  UtMutableUint16List *self = ut_object_get_data(object);
  return self->data_length;
}

static UtObject *ut_mutable_uint16_list_get_element(UtObject *object,
                                                    size_t index) {
  UtMutableUint16List *self = ut_object_get_data(object);
  return ut_uint16_new(self->data[index]);
}

static UtListFunctions list_functions = {
    .get_length = ut_mutable_uint16_list_get_length,
    .get_element = ut_mutable_uint16_list_get_element};

static const char *ut_mutable_uint16_list_get_type_name() {
  return "MutableUint16List";
}

static void ut_mutable_uint16_list_init(UtObject *object) {
  UtMutableUint16List *self = ut_object_get_data(object);
  self->data = NULL;
  self->data_length = 0;
}

static void ut_mutable_uint16_list_cleanup(UtObject *object) {
  UtMutableUint16List *self = ut_object_get_data(object);
  free(self->data);
}

static UtObjectFunctions object_functions = {
    .get_type_name = ut_mutable_uint16_list_get_type_name,
    .init = ut_mutable_uint16_list_init,
    .cleanup = ut_mutable_uint16_list_cleanup,
    .interfaces = {{&ut_uint16_list_id, &uint16_list_functions},
                   {&ut_mutable_list_id, &mutable_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_mutable_uint16_list_new() {
  return ut_object_new(sizeof(UtMutableUint16List), &object_functions);
}

void ut_mutable_uint16_list_append(UtObject *object, uint16_t data) {
  ut_mutable_uint16_list_append_block(object, &data, 1);
}

void ut_mutable_uint16_list_append_block(UtObject *object, const uint16_t *data,
                                         size_t data_length) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableUint16List *self = ut_object_get_data(object);
  ut_mutable_uint16_list_insert_block(object, self->data_length, data,
                                      data_length);
}

void ut_mutable_uint16_list_insert(UtObject *object, size_t index,
                                   uint16_t data) {
  ut_mutable_uint16_list_insert_block(object, index, &data, 1);
}

void ut_mutable_uint16_list_insert_block(UtObject *object, size_t index,
                                         const uint16_t *data,
                                         size_t data_length) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableUint16List *self = ut_object_get_data(object);

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

uint16_t *ut_mutable_uint16_list_get_data(UtObject *object) {
  assert(ut_object_is_type(object, &object_functions));
  UtMutableUint16List *self = ut_object_get_data(object);
  return self->data;
}

bool ut_object_is_mutable_uint16_list(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}