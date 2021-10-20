#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-object-private.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

int ut_uint8_list_id = 0;

UtObject *ut_uint8_list_new() { return ut_uint8_array_new(); }

const uint8_t *ut_uint8_list_get_data(UtObject *object) {
  UtUint8ListFunctions *uint8_list_functions =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_functions != NULL);
  return uint8_list_functions->get_data(object);
}

size_t ut_uint8_list_get_length(UtObject *object) {
  UtUint8ListFunctions *uint8_list_functions =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_functions != NULL);
  return uint8_list_functions->get_length(object);
}

uint8_t *ut_uint8_list_take_data(UtObject *object) {
  UtUint8ListFunctions *uint8_list_functions =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_functions != NULL);
  if (uint8_list_functions->take_data != NULL) {
    return uint8_list_functions->take_data(object);
  }

  const uint8_t *data = ut_uint8_list_get_data(object);
  size_t data_length = ut_uint8_list_get_length(object);
  uint8_t *data_copy = malloc(sizeof(uint8_t) * data_length);
  memcpy(data_copy, data, data_length);
  return data_copy;
}
