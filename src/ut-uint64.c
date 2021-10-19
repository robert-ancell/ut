#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-object-private.h"
#include "ut-uint64.h"

typedef struct {
  UtObject object;
  uint64_t value;
} UtUint64;

static char *ut_uint64_to_string(UtObject *object) {
  UtUint64 *self = (UtUint64 *)object;
  char string[11];
  snprintf(string, 11, "%lu", self->value);
  return strdup(string);
}

static int ut_uint64_equal(UtObject *object, UtObject *other) {
  UtUint64 *self = (UtUint64 *)object;
  if (!ut_object_is_uint64(other)) {
    return false;
  }
  UtUint64 *other_self = (UtUint64 *)other;
  return self->value = other_self->value;
}

static int ut_uint64_hash(UtObject *object) {
  UtUint64 *self = (UtUint64 *)object;
  return self->value;
}

static UtObjectFunctions object_functions = {.type_name = "UtUint64",
                                             .to_string = ut_uint64_to_string,
                                             .equal = ut_uint64_equal,
                                             .hash = ut_uint64_hash};

UtObject *ut_uint64_new(uint64_t value) {
  UtObject *object = ut_object_new(sizeof(UtUint64), &object_functions);
  UtUint64 *self = (UtUint64 *)object;
  self->value = value;
  return object;
}

uint64_t ut_uint64_get_value(UtObject *object) {
  assert(ut_object_is_uint64(object));
  UtUint64 *self = (UtUint64 *)object;
  return self->value;
}

bool ut_object_is_uint64(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
