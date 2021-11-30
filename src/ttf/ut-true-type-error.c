#include <assert.h>
#include <string.h>

#include "ut-error.h"
#include "ut-true-type-error.h"

typedef struct {
  UtObject object;
} UtTrueTypeError;

static char *ut_true_type_error_get_description(UtObject *object) {
  return strdup("TrueType Error");
}

static UtErrorInterface error_interface = {
    .get_description = ut_true_type_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtTrueTypeError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_true_type_error_new() {
  return ut_object_new(sizeof(UtTrueTypeError), &object_interface);
}

bool ut_object_is_true_type_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
