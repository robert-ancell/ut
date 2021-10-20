#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ut-mutable-string.h"
#include "ut-object-private.h"
#include "ut-object.h"
#include "ut-string.h"

UtObject *ut_object_new(size_t object_size, UtObjectFunctions *functions) {
  UtObject *object = malloc(object_size);
  object->functions = functions;
  object->ref_count = 1;

  if (functions->init != NULL) {
    functions->init(object);
  }

  return object;
}

bool ut_object_is_type(UtObject *object, UtObjectFunctions *functions) {
  return object->functions == functions;
}

const char *ut_object_get_type_name(UtObject *object) {
  return object->functions->type_name;
}

char *ut_object_to_string(UtObject *object) {
  if (object->functions->to_string != NULL) {
    return object->functions->to_string(object);
  }

  UtObjectRef string = ut_mutable_string_new("<");
  ut_mutable_string_append(string, ut_object_get_type_name(object));
  ut_mutable_string_append(string, ">");

  return strdup(ut_string_get_text(string));
}

bool ut_object_equal(UtObject *object, UtObject *other) {
  // Default equality is comparing an object against itself.
  if (object->functions->equal == NULL) {
    return object == other;
  }

  return object->functions->equal(object, other);
}

int ut_object_get_hash(UtObject *object) {
  // Default has is based off the memory address of the object.
  if (object->functions->hash == NULL) {
    return (intptr_t)object;
  }

  return object->functions->hash(object);
}

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
