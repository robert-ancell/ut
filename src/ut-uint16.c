#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ut-object-private.h"
#include "ut-uint16.h"

typedef struct {
  UtObject object;
  uint16_t value;
} UtUint16;

static char *ut_uint16_to_string(UtObject *object) {
  UtUint16 *self = (UtUint16 *)object;
  char string[6];
  snprintf(string, 6, "%d", self->value);
  return strdup(string);
}

static UtObjectFunctions object_functions = {.type_name = "Uint16",
                                             .to_string = ut_uint16_to_string};

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
