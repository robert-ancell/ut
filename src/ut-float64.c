#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-float64.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  double value;
} UtFloat64;

static char *ut_doubleo_string(UtObject *object) {
  UtFloat64 *self = (UtFloat64 *)object;
  char string[1024];
  snprintf(string, 1024, "%g", self->value);
  return strdup(string);
}

static bool ut_float64_equal(UtObject *object, UtObject *other) {
  UtFloat64 *self = (UtFloat64 *)object;
  if (!ut_object_is_float64(other)) {
    return false;
  }
  UtFloat64 *other_self = (UtFloat64 *)other;
  return self->value == other_self->value;
}

static int ut_float64_hash(UtObject *object) {
  UtFloat64 *self = (UtFloat64 *)object;
  return self->value;
}

static UtObjectFunctions object_functions = {.type_name = "UtFloat64",
                                             .to_string = ut_doubleo_string,
                                             .equal = ut_float64_equal,
                                             .hash = ut_float64_hash};

UtObject *ut_float64_new(double value) {
  UtObject *object = ut_object_new(sizeof(UtFloat64), &object_functions);
  UtFloat64 *self = (UtFloat64 *)object;
  self->value = value;
  return object;
}

double ut_float64_get_value(UtObject *object) {
  assert(ut_object_is_float64(object));
  UtFloat64 *self = (UtFloat64 *)object;
  return self->value;
}

bool ut_object_is_float64(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
