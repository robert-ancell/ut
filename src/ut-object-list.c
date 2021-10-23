#include <assert.h>

#include "ut-object-list.h"
#include "ut-object-private.h"

int ut_object_list_id = 0;

UtObject *ut_object_list_get_element(UtObject *object, size_t index) {
  UtObjectListInterface *object_list_interface =
      ut_object_get_interface(object, &ut_object_list_id);
  assert(object_list_interface != NULL);
  return object_list_interface->get_element(object, index);
}

bool ut_object_implements_object_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_object_list_id) != NULL;
}
