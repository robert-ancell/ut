#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint32-list.h"

int ut_uint32_list_id = 0;

uint32_t ut_uint32_list_get_element(UtObject *object, size_t index) {
  UtUint32ListInterface *uint32_list_interface =
      ut_object_get_interface(object, &ut_uint32_list_id);
  assert(uint32_list_interface != NULL);
  return uint32_list_interface->get_element(object, index);
}
