#include <assert.h>

#include "ut-list.h"
#include "ut-object-private.h"

int ut_list_id = 0;

size_t ut_list_get_length(UtObject *object) {
  UtListFunctions *list_functions =
      ut_object_get_interface(object, &ut_list_id);
  assert(list_functions != NULL);
  return list_functions->get_length(object);
}

UtObject *ut_list_get_element(UtObject *object, size_t index) {
  UtListFunctions *list_functions =
      ut_object_get_interface(object, &ut_list_id);
  assert(list_functions != NULL);
  return list_functions->get_element(object, index);
}
