#include <assert.h>

#include "ut-mutable-list.h"
#include "ut-object-private.h"

int ut_mutable_list_id = 0;

void ut_mutable_list_get_clear(UtObject *object) {
  UtMutableListFunctions *mutable_list_functions =
      ut_object_get_interface(object, &ut_mutable_list_id);
  assert(mutable_list_functions != NULL);
  return mutable_list_functions->clear(object);
}
