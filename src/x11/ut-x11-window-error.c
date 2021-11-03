#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-error.h"
#include "ut-x11-window-error.h"

typedef struct {
  UtObject object;
  uint32_t window;
} UtX11WindowError;

static void ut_x11_window_error_init(UtObject *object) {
  UtX11WindowError *self = (UtX11WindowError *)object;
  self->window = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11WindowError",
    .init = ut_x11_window_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_window_error_new(uint32_t window) {
  UtObject *object = ut_object_new(sizeof(UtX11WindowError), &object_interface);
  UtX11WindowError *self = (UtX11WindowError *)object;
  self->window = window;
  return object;
}

uint32_t ut_x11_window_error_get_window(UtObject *object) {
  assert(ut_object_is_x11_window_error(object));
  UtX11WindowError *self = (UtX11WindowError *)object;
  return self->window;
}

bool ut_object_is_x11_window_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
