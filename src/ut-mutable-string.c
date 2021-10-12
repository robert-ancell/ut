#include <assert.h>

#include "ut-mutable-string.h"
#include "ut-object-private.h"

int ut_mutable_string_id = 0;

void ut_mutable_string_clear(UtObject *object) {
  UtMutableStringFunctions *mutable_string_functions =
      ut_object_get_interface(object, &ut_mutable_string_id);
  assert(mutable_string_functions != NULL);
  mutable_string_functions->clear(object);
}

void ut_mutable_string_prepend(UtObject *object, const char *text) {
  UtMutableStringFunctions *mutable_string_functions =
      ut_object_get_interface(object, &ut_mutable_string_id);
  assert(mutable_string_functions != NULL);
  mutable_string_functions->prepend(object, text);
}

void ut_mutable_string_append(UtObject *object, const char *text) {
  UtMutableStringFunctions *mutable_string_functions =
      ut_object_get_interface(object, &ut_mutable_string_id);
  assert(mutable_string_functions != NULL);
  mutable_string_functions->append(object, text);
}
