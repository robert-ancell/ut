#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-int64.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  int64_t value;
} UtInt64;

static char *ut_int64_to_string(UtObject *object) {
  UtInt64 *self = (UtInt64 *)object;
  char string[11];
  snprintf(string, 11, "%li", self->value);
  return strdup(string);
}

static bool ut_int64_equal(UtObject *object, UtObject *other) {
  UtInt64 *self = (UtInt64 *)object;
  if (!ut_object_is_int64(other)) {
    return false;
  }
  UtInt64 *other_self = (UtInt64 *)other;
  return self->value == other_self->value;
}

static int ut_int64_hash(UtObject *object) {
  UtInt64 *self = (UtInt64 *)object;
  return self->value;
}

static UtObjectFunctions object_functions = {.type_name = "UtInt64",
                                             .to_string = ut_int64_to_string,
                                             .equal = ut_int64_equal,
                                             .hash = ut_int64_hash};

UtObject *ut_int64_new(int64_t value) {
  UtObject *object = ut_object_new(sizeof(UtInt64), &object_functions);
  UtInt64 *self = (UtInt64 *)object;
  self->value = value;
  return object;
}

int64_t ut_int64_get_value(UtObject *object) {
  assert(ut_object_is_int64(object));
  UtInt64 *self = (UtInt64 *)object;
  return self->value;
}

bool ut_object_is_int64(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
