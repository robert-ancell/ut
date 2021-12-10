#include <assert.h>

#include "ut-x11-event.h"
#include "ut-x11-map-notify.h"

typedef struct {
  UtObject object;
  uint32_t event;
  uint32_t window;
  bool override_redirect;
} UtX11MapNotify;

static void ut_x11_map_notify_init(UtObject *object) {
  UtX11MapNotify *self = (UtX11MapNotify *)object;
  self->event = 0;
  self->window = 0;
  self->override_redirect = false;
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11MapNotify",
    .init = ut_x11_map_notify_init,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_map_notify_new(uint32_t event, uint32_t window,
                                bool override_redirect) {
  UtObject *object = ut_object_new(sizeof(UtX11MapNotify), &object_interface);
  UtX11MapNotify *self = (UtX11MapNotify *)object;
  self->event = event;
  self->window = window;
  self->override_redirect = override_redirect;
  return object;
}

uint32_t ut_x11_map_notify_get_event(UtObject *object) {
  assert(ut_object_is_x11_map_notify(object));
  UtX11MapNotify *self = (UtX11MapNotify *)object;
  return self->event;
}

uint32_t ut_x11_map_notify_get_window(UtObject *object) {
  assert(ut_object_is_x11_map_notify(object));
  UtX11MapNotify *self = (UtX11MapNotify *)object;
  return self->window;
}

bool ut_x11_map_notify_get_override_redirect(UtObject *object) {
  assert(ut_object_is_x11_map_notify(object));
  UtX11MapNotify *self = (UtX11MapNotify *)object;
  return self->override_redirect;
}

bool ut_object_is_x11_map_notify(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
