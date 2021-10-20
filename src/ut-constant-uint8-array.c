#include <assert.h>
#include <stdlib.h>

#include "ut-constant-uint8-array.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-uint8-list.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  const uint8_t *data;
  size_t data_length;
} UtConstantUint8Array;

const uint8_t *ut_constant_uint8_array_get_data(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return self->data;
}

static size_t ut_constant_uint8_array_get_length(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return self->data_length;
}

static UtObject *ut_constant_uint8_array_get_element(UtObject *object,
                                                     size_t index) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  return ut_uint8_new(self->data[index]);
}

static UtUint8ListFunctions uint8_list_functions = {
    .get_data = ut_constant_uint8_array_get_data,
    .get_length = ut_constant_uint8_array_get_length};

static UtListFunctions list_functions = {
    .get_length = ut_constant_uint8_array_get_length,
    .get_element = ut_constant_uint8_array_get_element};

static void ut_constant_uint8_array_init(UtObject *object) {
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  self->data = NULL;
  self->data_length = 0;
}

static UtObjectFunctions object_functions = {
    .type_name = "UtConstantUint8Array",
    .init = ut_constant_uint8_array_init,
    .to_string = ut_list_to_string,
    .interfaces = {{&ut_uint8_list_id, &uint8_list_functions},
                   {&ut_list_id, &list_functions},
                   {NULL, NULL}}};

UtObject *ut_constant_uint8_array_new(const uint8_t *data, size_t data_length) {
  UtObject *object =
      ut_object_new(sizeof(UtConstantUint8Array), &object_functions);
  UtConstantUint8Array *self = (UtConstantUint8Array *)object;
  self->data = data;
  self->data_length = data_length;
  return object;
}

bool ut_object_is_constant_uint8_array(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
