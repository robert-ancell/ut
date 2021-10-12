#include <assert.h>

#include "ut-object-private.h"
#include "ut-string.h"

int ut_string_id = 0;

char *ut_string_get_text(UtObject *object) {
  UtStringFunctions *string_functions =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_functions != NULL);
  return string_functions->get_text(object);
}

UtObject *ut_string_get_code_points(UtObject *object) {
  UtStringFunctions *string_functions =
      ut_object_get_interface(object, &ut_string_id);
  assert(string_functions != NULL);
  return string_functions->get_code_points(object);
}
