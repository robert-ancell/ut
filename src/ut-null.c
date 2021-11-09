#include <string.h>

#include "ut-null.h"

typedef struct {
  UtObject object;
} UtNull;

static bool ut_null_equal(UtObject *object, UtObject *other) {
  return ut_object_is_null(other);
}

static int ut_null_hash(UtObject *object) { return 0; }

static UtObjectInterface object_interface = {
    .type_name = "UtNull", .equal = ut_null_equal, .hash = ut_null_hash};

UtObject *ut_null_new() {
  return ut_object_new(sizeof(UtNull), &object_interface);
}

bool ut_object_is_null(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
