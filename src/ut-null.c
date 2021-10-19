#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-null.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
} UtNull;

static char *ut_null_to_string(UtObject *object) { return strdup("null"); }

static int ut_null_equal(UtObject *object, UtObject *other) {
  return ut_object_is_null(other);
}

static int ut_null_hash(UtObject *object) { return 0; }

static UtObjectFunctions object_functions = {.type_name = "UtNull",
                                             .to_string = ut_null_to_string,
                                             .equal = ut_null_equal,
                                             .hash = ut_null_hash};

UtObject *ut_null_new() {
  return ut_object_new(sizeof(UtNull), &object_functions);
}

bool ut_object_is_null(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
