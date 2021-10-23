#include <assert.h>

#include "ut-file.h"
#include "ut-object-private.h"

int ut_file_id = 0;

void ut_file_open_read(UtObject *object) {
  UtFileInterface *file_interface =
      ut_object_get_interface(object, &ut_file_id);
  assert(file_interface != NULL);
  file_interface->open_read(object);
}

void ut_file_open_write(UtObject *object, bool create) {
  UtFileInterface *file_interface =
      ut_object_get_interface(object, &ut_file_id);
  assert(file_interface != NULL);
  file_interface->open_write(object, create);
}

void ut_file_close(UtObject *object) {
  UtFileInterface *file_interface =
      ut_object_get_interface(object, &ut_file_id);
  assert(file_interface != NULL);
  file_interface->close(object);
}

bool ut_object_implements_file(UtObject *object) {
  return ut_object_get_interface(object, &ut_file_id) != NULL;
}
