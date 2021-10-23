#include <assert.h>

#include "ut-error.h"
#include "ut-general-error.h"
#include "ut-object-private.h"

int ut_error_id = 0;

UtObject *ut_error_new(const char *description) {
  return ut_general_error_new(description);
}

char *ut_error_get_description(UtObject *object) {
  UtErrorFunctions *error_functions =
      ut_object_get_interface(object, &ut_error_id);
  assert(error_functions != NULL);
  return error_functions->get_description(object);
}

bool ut_object_implements_error(UtObject *object) {
  return ut_object_get_interface(object, &ut_error_id) != NULL;
}
