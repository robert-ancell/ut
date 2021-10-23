#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint16-list.h"

int ut_uint16_list_id = 0;

const uint16_t *ut_uint16_list_get_data(UtObject *object) {
  UtUint16ListInterface *uint16_list_interface =
      ut_object_get_interface(object, &ut_uint16_list_id);
  assert(uint16_list_interface != NULL);
  return uint16_list_interface->get_data(object);
}
