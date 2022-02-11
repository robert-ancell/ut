#include <assert.h>

#include "ut-x11-error.h"
#include "ut-x11-gcontext-error.h"

typedef struct {
  UtObject object;
  uint32_t gc;
} UtX11GContextError;

static void ut_x11_gcontext_error_init(UtObject *object) {
  UtX11GContextError *self = (UtX11GContextError *)object;
  self->gc = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11GContextError",
    .init = ut_x11_gcontext_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}, {NULL, NULL}}};

UtObject *ut_x11_gcontext_error_new(uint32_t gc) {
  UtObject *object =
      ut_object_new(sizeof(UtX11GContextError), &object_interface);
  UtX11GContextError *self = (UtX11GContextError *)object;
  self->gc = gc;
  return object;
}

uint32_t ut_x11_gcontext_error_get_gc(UtObject *object) {
  assert(ut_object_is_x11_gcontext_error(object));
  UtX11GContextError *self = (UtX11GContextError *)object;
  return self->gc;
}

bool ut_object_is_x11_gcontext_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
