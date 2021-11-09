#include <assert.h>

#include "ut-error.h"
#include "ut-general-error.h"

int ut_error_id = 0;

UtObject *ut_error_new(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  UtObject *error = ut_general_error_new_valist(format, ap);
  va_end(ap);

  return error;
}

char *ut_error_get_description(UtObject *object) {
  UtErrorInterface *error_interface =
      ut_object_get_interface(object, &ut_error_id);
  assert(error_interface != NULL);
  return error_interface->get_description(object);
}

bool ut_object_implements_error(UtObject *object) {
  return ut_object_get_interface(object, &ut_error_id) != NULL;
}
