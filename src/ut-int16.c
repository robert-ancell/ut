#include <assert.h>
#include <stdio.h>

#include "ut-cstring.h"
#include "ut-int16.h"

typedef struct {
  UtObject object;
  int16_t value;
} UtInt16;

static char *ut_int16_to_string(UtObject *object) {
  UtInt16 *self = (UtInt16 *)object;
  return ut_cstring_new_printf("<int16>(%d)", self->value);
}

static bool ut_int16_equal(UtObject *object, UtObject *other) {
  UtInt16 *self = (UtInt16 *)object;
  if (!ut_object_is_int16(other)) {
    return false;
  }
  UtInt16 *other_self = (UtInt16 *)other;
  return self->value == other_self->value;
}

static int ut_int16_hash(UtObject *object) {
  UtInt16 *self = (UtInt16 *)object;
  return self->value;
}

static UtObjectInterface object_interface = {.type_name = "UtInt16",
                                             .to_string = ut_int16_to_string,
                                             .equal = ut_int16_equal,
                                             .hash = ut_int16_hash};

UtObject *ut_int16_new(int16_t value) {
  UtObject *object = ut_object_new(sizeof(UtInt16), &object_interface);
  UtInt16 *self = (UtInt16 *)object;
  self->value = value;
  return object;
}

int16_t ut_int16_get_value(UtObject *object) {
  assert(ut_object_is_int16(object));
  UtInt16 *self = (UtInt16 *)object;
  return self->value;
}

bool ut_object_is_int16(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
