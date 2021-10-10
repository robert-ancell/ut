#include <assert.h>
#include <stdlib.h>

#include "ut-object.h"

struct _UtObject {
  UtObjectFunctions *functions;
  int ref_count;
};

UtObject *ut_object_new(size_t data_size, UtObjectFunctions *functions) {
  UtObject *object = malloc(sizeof(UtObject) + data_size);
  object->functions = functions;
  object->ref_count = 1;
  return object;
}

void *ut_object_get_data(UtObject *object) { return object + sizeof(UtObject); }

UtObject *ut_object_ref(UtObject *object) {
  assert(object->ref_count > 0);

  object->ref_count++;
  return object;
}

void ut_object_unref(UtObject *object) {
  assert(object->ref_count > 0);

  object->ref_count--;
  if (object->ref_count == 0) {
    object->functions->cleanup(object);
    free(object);
  }
}

void *ut_object_get_interface(UtObject *object, void *interface_id) {
  return object->functions->get_interface(object, interface_id);
}
