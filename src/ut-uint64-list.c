#include <assert.h>

#include "ut-uint64-list.h"

int ut_uint64_list_id = 0;

uint64_t ut_uint64_list_get_element(UtObject *object, size_t index) {
  UtUint64ListInterface *uint64_list_interface =
      ut_object_get_interface(object, &ut_uint64_list_id);
  assert(uint64_list_interface != NULL);
  return uint64_list_interface->get_element(object, index);
}
