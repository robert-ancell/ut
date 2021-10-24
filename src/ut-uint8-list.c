#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-object-private.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

int ut_uint8_list_id = 0;

UtObject *ut_uint8_list_new() { return ut_uint8_array_new(); }

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->get_element(object, index);
}

uint8_t *ut_uint8_list_take_data(UtObject *object) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->take_data(object);
}
