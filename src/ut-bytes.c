#include <stdlib.h>

#include "ut-bytes.h"
#include "ut-list.h"

typedef struct {
  uint8_t *data;
  size_t length;
} UtBytes;

size_t ut_bytes_get_length(UtObject *object) {
  UtBytes *self = ut_object_get_data(object);
  return self->length;
}

static UtListFunctions list_functions = {.get_length = ut_bytes_get_length};

static void ut_bytes_init(UtObject *object) {}

static void ut_bytes_cleanup(UtObject *object) {
  UtBytes *self = ut_object_get_data(object);
  free(self->data);
}

static void *ut_bytes_get_interface(UtObject *object, void *interface_id) {
  if (interface_id == &ut_list_id) {
    return &list_functions;
  }

  return NULL;
}

static UtObjectFunctions object_functions = {.init = ut_bytes_init,
                                             .cleanup = ut_bytes_cleanup,
                                             .get_interface =
                                                 ut_bytes_get_interface};

UtObject *ut_bytes_new(const uint8_t *data, size_t length) {
  UtObject *object = ut_object_new(sizeof(UtBytes), &object_functions);
  UtBytes *self = ut_object_get_data(object);
  self->data = malloc(sizeof(uint8_t) * length);
  for (size_t i = 0; i < length; i++) {
    self->data[i] = data[i];
  }
  self->length = length;
  return object;
}

const uint8_t *ut_bytes_get_data(UtObject *object) {
  UtBytes *self = ut_object_get_data(object);
  return self->data;
}
