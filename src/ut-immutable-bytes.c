#include <assert.h>
#include <stdlib.h>

#include "ut-bytes.h"
#include "ut-immutable-bytes.h"
#include "ut-list.h"

typedef struct {
  uint8_t *data;
  size_t data_length;
} UtImmutableBytes;

const uint8_t *ut_immutable_bytes_get_data(UtObject *object) {
  UtImmutableBytes *self = ut_object_get_data(object);
  return self->data;
}

static UtBytesFunctions bytes_functions = {.get_data =
                                               ut_immutable_bytes_get_data};

size_t ut_immutable_bytes_get_length(UtObject *object) {
  UtImmutableBytes *self = ut_object_get_data(object);
  return self->data_length;
}

static UtListFunctions list_functions = {.get_length =
                                             ut_immutable_bytes_get_length};

static void ut_immutable_bytes_init(UtObject *object) {}

static void ut_immutable_bytes_cleanup(UtObject *object) {
  UtImmutableBytes *self = ut_object_get_data(object);
  free(self->data);
}

static UtObjectFunctions object_functions = {
    .init = ut_immutable_bytes_init,
    .cleanup = ut_immutable_bytes_cleanup,
    {{&ut_bytes_id, &bytes_functions}, {&ut_list_id, &list_functions}}};

UtObject *ut_immutable_bytes_new(const uint8_t *data, size_t data_length) {
  UtObject *object = ut_object_new(sizeof(UtImmutableBytes), &object_functions);
  UtImmutableBytes *self = ut_object_get_data(object);
  self->data = malloc(sizeof(uint8_t) * data_length);
  for (size_t i = 0; i < data_length; i++) {
    self->data[i] = data[i];
  }
  self->data_length = data_length;
  return object;
}

bool ut_object_is_immutable_bytes(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
