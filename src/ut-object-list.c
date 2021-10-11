#include <assert.h>

#include "ut-object-list.h"
#include "ut-object-private.h"

int ut_object_list_id = 0;

UtObject *ut_object_list_get_element(UtObject *object, size_t index) {
  UtObjectListFunctions *object_list_functions =
      ut_object_get_interface(object, &ut_object_list_id);
  assert(object_list_functions != NULL);
  return object_list_functions->get_element(object, index);
}
