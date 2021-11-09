#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "ut-constant-uint8-array.h"
#include "ut-list.h"
#include "ut-string.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  const uint8_t *data;
  size_t data_length;
} UtConstantUint8Array;

uint8_t ut_constant_uint8_array_get_element(UtObject *object, size_t index) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return self->data[index];
}

static size_t ut_constant_uint8_array_get_length(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return self->data_length;
}

static UtObject *ut_constant_uint8_array_get_element_object(UtObject *object,
                                                            size_t index) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return ut_uint8_new(self->data[index]);
}

static UtObject *ut_constant_uint8_array_copy(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  UtObject *copy = ut_uint8_array_new();
  ut_uint8_list_append_block(copy, self->data, self->data_length);
  return copy;
}

static UtUint8ListInterface uint8_list_interface = {
    .get_element = ut_constant_uint8_array_get_element};

static UtListInterface list_interface = {
    .get_length = ut_constant_uint8_array_get_length,
    .get_element = ut_constant_uint8_array_get_element_object,
    .copy = ut_constant_uint8_array_copy};

static void ut_constant_uint8_array_init(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static char *ut_constant_uint8_array_to_string(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
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

static UtObjectInterface object_interface = {
    .type_name = "UtConstantUint8Array",
    .init = ut_constant_uint8_array_init,
    .to_string = ut_constant_uint8_array_to_string,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_constant_uint8_array_new(const uint8_t *data, size_t data_length) {
  UtObject *object =
      ut_object_new(sizeof(UtConstantUint8Array), &object_interface);
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  self->data = data;
  self->data_length = data_length;
  return object;
}

const uint8_t *ut_constant_uint8_array_get_data(UtObject *object) {
  assert(ut_object_is_constant_uint8_array(object));
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return self->data;
}

bool ut_object_is_constant_uint8_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
