#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-error.h"

int ut_x11_error_id = 0;

bool ut_object_implements_x11_error(UtObject *object) {
  return ut_object_get_interface(object, &ut_x11_error_id) != NULL;
}
