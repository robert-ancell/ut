#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint32-list.h"

int ut_uint32_list_id = 0;

const uint32_t *ut_uint32_list_get_data(UtObject *object) {
  UtUint32ListFunctions *uint32_list_functions =
      ut_object_get_interface(object, &ut_uint32_list_id);
  assert(uint32_list_functions != NULL);
  return uint32_list_functions->get_data(object);
}