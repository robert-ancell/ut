#include <assert.h>

#include "ut-x11-event.h"
#include "ut-x11-key-release.h"

typedef struct {
  UtObject object;
  uint32_t window;
  uint8_t keycode;
  int16_t x;
  int16_t y;
} UtX11KeyRelease;

static void ut_x11_key_release_init(UtObject *object) {
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  self->window = 0;
  self->keycode = 0;
  self->x = 0;
  self->y = 0;
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11KeyRelease",
    .init = ut_x11_key_release_init,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_key_release_new(uint32_t window, uint8_t keycode, int16_t x,
                                 int16_t y) {
  UtObject *object = ut_object_new(sizeof(UtX11KeyRelease), &object_interface);
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  self->window = window;
  self->keycode = keycode;
  self->x = x;
  self->y = y;
  return object;
}

uint32_t ut_x11_key_release_get_window(UtObject *object) {
  assert(ut_object_is_x11_key_release(object));
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  return self->window;
}

uint8_t ut_x11_key_release_get_keycode(UtObject *object) {
  assert(ut_object_is_x11_key_release(object));
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  return self->keycode;
}

int16_t ut_x11_key_release_get_x(UtObject *object) {
  assert(ut_object_is_x11_key_release(object));
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  return self->x;
}

int16_t ut_x11_key_release_get_y(UtObject *object) {
  assert(ut_object_is_x11_key_release(object));
  UtX11KeyRelease *self = (UtX11KeyRelease *)object;
  return self->y;
}

bool ut_object_is_x11_key_release(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
