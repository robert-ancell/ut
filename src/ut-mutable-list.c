#include <assert.h>

#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-object-private.h"

int ut_mutable_list_id = 0;

void ut_mutable_list_append(UtObject *object, UtObject *item) {
  size_t length = ut_list_get_length(object);
  ut_mutable_list_insert(object, length, item);
}

void ut_mutable_list_prepend(UtObject *object, UtObject *item) {
  ut_mutable_list_insert(object, 0, item);
}

void ut_mutable_list_insert(UtObject *object, size_t index, UtObject *item) {
  UtMutableListFunctions *mutable_list_functions =
      ut_object_get_interface(object, &ut_mutable_list_id);
  assert(mutable_list_functions != NULL);
  mutable_list_functions->insert(object, index, item);
}

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
