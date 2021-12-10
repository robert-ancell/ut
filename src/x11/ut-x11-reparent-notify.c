#include <assert.h>

#include "ut-x11-event.h"
#include "ut-x11-reparent-notify.h"

typedef struct {
  UtObject object;
  uint32_t event;
  uint32_t window;
  uint32_t parent;
  int16_t x;
  int16_t y;
  bool override_redirect;
} UtX11ReparentNotify;

static void ut_x11_reparent_notify_init(UtObject *object) {
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  self->event = 0;
  self->window = 0;
  self->parent = 0;
  self->x = 0;
  self->y = 0;
  self->override_redirect = false;
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11ReparentNotify",
    .init = ut_x11_reparent_notify_init,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_reparent_notify_new(uint32_t event, uint32_t window,
                                     uint32_t parent, int16_t x, int16_t y,
                                     bool override_redirect) {
  UtObject *object =
      ut_object_new(sizeof(UtX11ReparentNotify), &object_interface);
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  self->event = event;
  self->window = window;
  self->parent = parent;
  self->x = x;
  self->y = y;
  self->override_redirect = override_redirect;
  return object;
}

uint32_t ut_x11_reparent_notify_get_event(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->event;
}

uint32_t ut_x11_reparent_notify_get_window(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->window;
}

uint32_t ut_x11_reparent_notify_get_parent(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->parent;
}

int16_t ut_x11_reparent_notify_get_x(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->x;
}

int16_t ut_x11_reparent_notify_get_y(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->y;
}

bool ut_x11_reparent_notify_get_override_redirect(UtObject *object) {
  assert(ut_object_is_x11_reparent_notify(object));
  UtX11ReparentNotify *self = (UtX11ReparentNotify *)object;
  return self->override_redirect;
}

bool ut_object_is_x11_reparent_notify(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
