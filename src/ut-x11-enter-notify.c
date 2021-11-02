#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-enter-notify.h"

typedef struct {
  UtObject object;
  uint32_t window;
  int16_t x;
  int16_t y;
} UtX11EnterNotify;

static void ut_x11_enter_notify_init(UtObject *object) {
  UtX11EnterNotify *self = (UtX11EnterNotify *)object;
  self->window = 0;
  self->x = 0;
  self->y = 0;
}

static UtObjectInterface object_interface = {.type_name = "UtX11EnterNotify",
                                             .init = ut_x11_enter_notify_init,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_enter_notify_new(uint32_t window, int16_t x, int16_t y) {
  UtObject *object = ut_object_new(sizeof(UtX11EnterNotify), &object_interface);
  UtX11EnterNotify *self = (UtX11EnterNotify *)object;
  self->window = window;
  self->x = x;
  self->y = y;
  return object;
}

uint32_t ut_x11_enter_notify_get_window(UtObject *object) {
  assert(ut_object_is_x11_enter_notify(object));
  UtX11EnterNotify *self = (UtX11EnterNotify *)object;
  return self->window;
}

int16_t ut_x11_enter_notify_get_x(UtObject *object) {
  assert(ut_object_is_x11_enter_notify(object));
  UtX11EnterNotify *self = (UtX11EnterNotify *)object;
  return self->x;
}

int16_t ut_x11_enter_notify_get_y(UtObject *object) {
  assert(ut_object_is_x11_enter_notify(object));
  UtX11EnterNotify *self = (UtX11EnterNotify *)object;
  return self->y;
}

bool ut_object_is_x11_enter_notify(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
