#include <assert.h>

#include "ut-bytes.h"

int ut_bytes_id = 0;

const uint8_t *ut_bytes_get_data(UtObject *object) {
  UtBytesFunctions *bytes_functions =
      ut_object_get_interface(object, &ut_bytes_id);
  assert(bytes_functions != NULL);
  return bytes_functions->get_data(object);
}
