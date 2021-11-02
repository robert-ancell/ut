#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-configure-notify.h"

typedef struct {
  UtObject object;
  uint32_t window;
  int16_t x;
  int16_t y;
  uint16_t width;
  uint16_t height;
} UtX11ConfigureNotify;

static void ut_x11_configure_notify_init(UtObject *object) {
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  self->window = 0;
  self->x = 0;
  self->y = 0;
  self->width = 0;
  self->height = 0;
}

static UtObjectInterface object_interface = {
    .type_name = "UtX11ConfigureNotify",
    .init = ut_x11_configure_notify_init,
    .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_configure_notify_new(uint32_t window, int16_t x, int16_t y,
                                      uint16_t width, uint16_t height) {
  UtObject *object =
      ut_object_new(sizeof(UtX11ConfigureNotify), &object_interface);
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  self->window = window;
  self->x = x;
  self->y = y;
  self->width = width;
  self->height = height;
  return object;
}

uint32_t ut_x11_configure_notify_get_window(UtObject *object) {
  assert(ut_object_is_x11_configure_notify(object));
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  return self->window;
}

int16_t ut_x11_configure_notify_get_x(UtObject *object) {
  assert(ut_object_is_x11_configure_notify(object));
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  return self->x;
}

int16_t ut_x11_configure_notify_get_y(UtObject *object) {
  assert(ut_object_is_x11_configure_notify(object));
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  return self->y;
}

uint16_t ut_x11_configure_notify_get_width(UtObject *object) {
  assert(ut_object_is_x11_configure_notify(object));
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  return self->width;
}

uint16_t ut_x11_configure_notify_get_height(UtObject *object) {
  assert(ut_object_is_x11_configure_notify(object));
  UtX11ConfigureNotify *self = (UtX11ConfigureNotify *)object;
  return self->height;
}

bool ut_object_is_x11_configure_notify(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
