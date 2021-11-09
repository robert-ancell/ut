#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ut-object.h"
#include "ut-string.h"

UtObject *ut_object_new(size_t object_size, UtObjectInterface *interface) {
  UtObject *object = malloc(object_size);
  object->interface = interface;
  object->ref_count = 1;

  if (interface->init != NULL) {
    interface->init(object);
  }

  return object;
}

bool ut_object_is_type(UtObject *object, UtObjectInterface *interface) {
  return object->interface == interface;
}

const char *ut_object_get_type_name(UtObject *object) {
  return object->interface->type_name;
}

char *ut_object_to_string(UtObject *object) {
  if (object->interface->to_string != NULL) {
    return object->interface->to_string(object);
  }

  UtObjectRef string = ut_string_new("<");
  ut_string_append(string, ut_object_get_type_name(object));
  ut_string_append(string, ">");

  return ut_string_take_text(string);
}

bool ut_object_equal(UtObject *object, UtObject *other) {
  // Default equality is comparing an object against itself.
  if (object->interface->equal == NULL) {
    return object == other;
  }

  return object->interface->equal(object, other);
}

int ut_object_get_hash(UtObject *object) {
  // Default has is based off the memory address of the object.
  if (object->interface->hash == NULL) {
    return (intptr_t)object;
  }

  return object->interface->hash(object);
}

UtObject *ut_object_ref(UtObject *object) {
  if (object == NULL) {
    return NULL;
  }

  assert(object->ref_count > 0);

  object->ref_count++;
  return object;
}

void ut_object_unref(UtObject *object) {
  if (object == NULL) {
    return;
  }

  assert(object->ref_count > 0);

  object->ref_count--;
  if (object->ref_count == 0) {
    if (object->interface->cleanup != NULL) {
      object->interface->cleanup(object);
    }
    free(object);
  }
}

void *ut_object_get_interface(UtObject *object, void *interface_id) {
  for (int i = 0; object->interface->interfaces[i].interface_id != NULL; i++) {
    if (object->interface->interfaces[i].interface_id == interface_id) {
      return object->interface->interfaces[i].interface;
    }
  }

  return NULL;
}
