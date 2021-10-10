#include <stdlib.h>

#include "ut-object.h"

struct _UtObject {
  UtObjectFunctions *functions;
  int ref_count;
};

UtObject *ut_object_new(UtObjectFunctions *functions) {
  UtObject *object = malloc(sizeof(UtObject));
  object->functions = functions;
  object->ref_count = 0;
  return object;
}

UtObject *ut_object_ref(UtObject *object) {
  object->ref_count++;
  return object;
}

void ut_object_unref(UtObject *object) {
  // FIXME: assert ref_count > 0
  object->ref_count--;
  if (object->ref_count == 0) {
    object->functions->cleanup(object);
    free(object);
  }
}

void *ut_object_get_interface(UtObject *object, void *interface_id) {
  return NULL;
}
