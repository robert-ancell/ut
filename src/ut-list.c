#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-mutable-string.h"
#include "ut-object-private.h"
#include "ut-string.h"

int ut_list_id = 0;

size_t ut_list_get_length(UtObject *object) {
  UtListFunctions *list_functions =
      ut_object_get_interface(object, &ut_list_id);
  assert(list_functions != NULL);
  return list_functions->get_length(object);
}

UtObject *ut_list_get_element(UtObject *object, size_t index) {
  UtListFunctions *list_functions =
      ut_object_get_interface(object, &ut_list_id);
  assert(list_functions != NULL);
  return list_functions->get_element(object, index);
}

char *ut_list_to_string(UtObject *object) {
  UtObject *string = ut_mutable_string_new("[");
  for (size_t i = 0; i < ut_list_get_length(object); i++) {
    UtObject *item = ut_list_get_element(object, i);

    if (i != 0) {
      ut_mutable_string_append(string, ", ");
    }

    char *value_string = ut_object_to_string(item);
    ut_mutable_string_append(string, value_string);
    free(value_string);

    ut_object_unref(item);
  }
  ut_mutable_string_append(string, "]");

  char *result = strdup(ut_string_get_text(string));
  ut_object_unref(string);
  return result;
}

bool ut_object_implements_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_list_id) != NULL;
}
