#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-object-private.h"
#include "ut-object.h"

struct _UtObject {
  UtObjectFunctions *functions;
  int ref_count;
  void *data;
};

UtObject *ut_object_new(size_t data_size, UtObjectFunctions *functions) {
  UtObject *object = malloc(sizeof(UtObject));
  object->functions = functions;
  object->ref_count = 1;
  object->data = malloc(data_size);

  if (functions->init != NULL) {
    functions->init(object);
  }

  return object;
}

bool ut_object_is_type(UtObject *object, UtObjectFunctions *functions) {
  return object->functions == functions;
}

const char *ut_object_get_type_name(UtObject *object) {
  return object->functions->get_type_name();
}

char *ut_object_to_string(UtObject *object) {
  if (object->functions->to_string != NULL) {
    return object->functions->to_string(object);
  }

  return strdup(ut_object_get_type_name(object));
}

void *ut_object_get_data(UtObject *object) { return object->data; }

UtObject *ut_object_ref(UtObject *object) {
  assert(object->ref_count > 0);

  object->ref_count++;
  return object;
}

void ut_object_unref(UtObject *object) {
  assert(object->ref_count > 0);

  object->ref_count--;
  if (object->ref_count == 0) {
    if (object->functions->cleanup != NULL) {
      object->functions->cleanup(object);
    }
    free(object);
  }
}

void *ut_object_get_interface(UtObject *object, void *interface_id) {
  for (int i = 0; object->functions->interfaces[i].interface_id != NULL; i++) {
    if (object->functions->interfaces[i].interface_id == interface_id) {
      return object->functions->interfaces[i].functions;
    }
  }

  return NULL;
}
