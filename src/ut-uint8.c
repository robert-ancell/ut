#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint8.h"

typedef struct {
  UtObject object;
  uint8_t value;
} UtUint8;

static UtObjectFunctions object_functions = {};

UtObject *ut_uint8_new(uint8_t value) {
  UtObject *object = ut_object_new(sizeof(UtUint8), &object_functions);
  UtUint8 *self = (UtUint8 *)object;
  self->value = value;
  return object;
}

uint8_t ut_uint8_get_value(UtObject *object) {
  assert(ut_object_is_uint8(object));
  UtUint8 *self = (UtUint8 *)object;
  return self->value;
}

bool ut_object_is_uint8(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
