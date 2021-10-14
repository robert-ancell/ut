#include <assert.h>

#include "ut-mutable-list.h"
#include "ut-object-private.h"

int ut_mutable_list_id = 0;

void ut_mutable_list_clear(UtObject *object) {
  ut_mutable_list_resize(object, 0);
}

void ut_mutable_list_resize(UtObject *object, size_t length) {
  UtMutableListFunctions *mutable_list_functions =
      ut_object_get_interface(object, &ut_mutable_list_id);
  assert(mutable_list_functions != NULL);
  mutable_list_functions->resize(object, length);
}

bool ut_object_implements_mutable_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_mutable_list_id) != NULL;
}
