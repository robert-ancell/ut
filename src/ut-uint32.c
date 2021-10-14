#include <assert.h>

#include "ut-object-private.h"
#include "ut-uint32.h"

typedef struct {
  UtObject object;
  uint32_t value;
} UtUint32;

static UtObjectFunctions object_functions = {};

UtObject *ut_uint32_new(uint32_t value) {
  UtObject *object = ut_object_new(sizeof(UtUint32), &object_functions);
  UtUint32 *self = (UtUint32 *)object;
  self->value = value;
  return object;
}

uint32_t ut_uint32_get_value(UtObject *object) {
  assert(ut_object_is_uint32(object));
  UtUint32 *self = (UtUint32 *)object;
  return self->value;
}

bool ut_object_is_uint32(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
