#include <assert.h>

#include "ut-x11-drawable-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
  uint32_t drawable;
} UtX11DrawableError;

static void ut_x11_drawable_error_init(UtObject *object) {
  UtX11DrawableError *self = (UtX11DrawableError *)object;
  self->drawable = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11DrawableError",
    .init = ut_x11_drawable_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_drawable_error_new(uint32_t drawable) {
  UtObject *object =
      ut_object_new(sizeof(UtX11DrawableError), &object_interface);
  UtX11DrawableError *self = (UtX11DrawableError *)object;
  self->drawable = drawable;
  return object;
}

uint32_t ut_x11_drawable_error_get_drawable(UtObject *object) {
  assert(ut_object_is_x11_drawable_error(object));
  UtX11DrawableError *self = (UtX11DrawableError *)object;
  return self->drawable;
}

bool ut_object_is_x11_drawable_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
