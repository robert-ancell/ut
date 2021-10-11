#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint8-list.h"

int ut_uint8_list_id = 0;

const uint8_t *ut_uint8_list_get_data(UtObject *object) {
  UtUint8ListFunctions *uint8_list_functions =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_functions != NULL);
  return uint8_list_functions->get_data(object);
}
