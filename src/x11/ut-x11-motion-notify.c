#include <assert.h>

#include "ut-x11-event.h"
#include "ut-x11-motion-notify.h"

typedef struct {
  UtObject object;
  uint32_t window;
  int16_t x;
  int16_t y;
} UtX11MotionNotify;

static void ut_x11_motion_notify_init(UtObject *object) {
  UtX11MotionNotify *self = (UtX11MotionNotify *)object;
  self->window = 0;
  self->x = 0;
  self->y = 0;
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11MotionNotify",
    .init = ut_x11_motion_notify_init,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_motion_notify_new(uint32_t window, int16_t x, int16_t y) {
  UtObject *object =
      ut_object_new(sizeof(UtX11MotionNotify), &object_interface);
  UtX11MotionNotify *self = (UtX11MotionNotify *)object;
  self->window = window;
  self->x = x;
  self->y = y;
  return object;
}

uint32_t ut_x11_motion_notify_get_window(UtObject *object) {
  assert(ut_object_is_x11_motion_notify(object));
  UtX11MotionNotify *self = (UtX11MotionNotify *)object;
  return self->window;
}

int16_t ut_x11_motion_notify_get_x(UtObject *object) {
  assert(ut_object_is_x11_motion_notify(object));
  UtX11MotionNotify *self = (UtX11MotionNotify *)object;
  return self->x;
}

int16_t ut_x11_motion_notify_get_y(UtObject *object) {
  assert(ut_object_is_x11_motion_notify(object));
  UtX11MotionNotify *self = (UtX11MotionNotify *)object;
  return self->y;
}

bool ut_object_is_x11_motion_notify(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
