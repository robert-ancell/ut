#include <assert.h>
#include <stdio.h>

#include "ut-cstring.h"
#include "ut-int32.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  int32_t value;
} UtInt32;

static char *ut_int32_to_string(UtObject *object) {
  UtInt32 *self = (UtInt32 *)object;
  return ut_cstring_new_printf("<int32>(%d)", self->value);
}

static bool ut_int32_equal(UtObject *object, UtObject *other) {
  UtInt32 *self = (UtInt32 *)object;
  if (!ut_object_is_int32(other)) {
    return false;
  }
  UtInt32 *other_self = (UtInt32 *)other;
  return self->value == other_self->value;
}

static int ut_int32_hash(UtObject *object) {
  UtInt32 *self = (UtInt32 *)object;
  return self->value;
}

static UtObjectInterface object_interface = {.type_name = "UtInt32",
                                             .to_string = ut_int32_to_string,
                                             .equal = ut_int32_equal,
                                             .hash = ut_int32_hash};

UtObject *ut_int32_new(int32_t value) {
  UtObject *object = ut_object_new(sizeof(UtInt32), &object_interface);
  UtInt32 *self = (UtInt32 *)object;
  self->value = value;
  return object;
}

int32_t ut_int32_get_value(UtObject *object) {
  assert(ut_object_is_int32(object));
  UtInt32 *self = (UtInt32 *)object;
  return self->value;
}

bool ut_object_is_int32(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
