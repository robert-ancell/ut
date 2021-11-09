#include <assert.h>

#include "ut-x11-error.h"
#include "ut-x11-value-error.h"

typedef struct {
  UtObject object;
  uint32_t value;
} UtX11ValueError;

static void ut_x11_value_error_init(UtObject *object) {
  UtX11ValueError *self = (UtX11ValueError *)object;
  self->value = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11ValueError",
    .init = ut_x11_value_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_value_error_new(uint32_t value) {
  UtObject *object = ut_object_new(sizeof(UtX11ValueError), &object_interface);
  UtX11ValueError *self = (UtX11ValueError *)object;
  self->value = value;
  return object;
}

uint32_t ut_x11_value_error_get_value(UtObject *object) {
  assert(ut_object_is_x11_value_error(object));
  UtX11ValueError *self = (UtX11ValueError *)object;
  return self->value;
}

bool ut_object_is_x11_value_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
