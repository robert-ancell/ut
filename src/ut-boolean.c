#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-boolean.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  bool value;
} UtBoolean;

static char *ut_boolean_to_string(UtObject *object) {
  UtBoolean *self = (UtBoolean *)object;
  return strdup(self->value ? "true" : "false");
}

static bool ut_boolean_equal(UtObject *object, UtObject *other) {
  UtBoolean *self = (UtBoolean *)object;
  if (!ut_object_is_boolean(other)) {
    return false;
  }
  UtBoolean *other_self = (UtBoolean *)other;
  return self->value = other_self->value;
}

static int ut_boolean_hash(UtObject *object) {
  UtBoolean *self = (UtBoolean *)object;
  return self->value;
}

static UtObjectInterface object_interface = {.type_name = "UtBoolean",
                                             .to_string = ut_boolean_to_string,
                                             .equal = ut_boolean_equal,
                                             .hash = ut_boolean_hash};

UtObject *ut_boolean_new(bool value) {
  UtObject *object = ut_object_new(sizeof(UtBoolean), &object_interface);
  UtBoolean *self = (UtBoolean *)object;
  self->value = value;
  return object;
}

bool ut_boolean_get_value(UtObject *object) {
  assert(ut_object_is_boolean(object));
  UtBoolean *self = (UtBoolean *)object;
  return self->value;
}

bool ut_object_is_boolean(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
