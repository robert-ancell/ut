#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint16.h"

typedef struct {
  UtObject object;
  uint16_t value;
} UtUint16;

static UtObjectFunctions object_functions = {};

UtObject *ut_uint16_new(uint16_t value) {
  UtObject *object = ut_object_new(sizeof(UtUint16), &object_functions);
  UtUint16 *self = (UtUint16 *)object;
  self->value = value;
  return object;
}

uint16_t ut_uint16_get_value(UtObject *object) {
  assert(ut_object_is_uint16(object));
  UtUint16 *self = (UtUint16 *)object;
  return self->value;
}

bool ut_object_is_uint16(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
