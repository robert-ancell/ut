#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-key-press.h"

typedef struct {
  UtObject object;
  uint32_t window;
  uint8_t keycode;
  int16_t x;
  int16_t y;
} UtX11KeyPress;

static void ut_x11_key_press_init(UtObject *object) {
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  self->window = 0;
  self->keycode = 0;
  self->x = 0;
  self->y = 0;
}

static UtObjectInterface object_interface = {.type_name = "UtX11KeyPress",
                                             .init = ut_x11_key_press_init,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_key_press_new(uint32_t window, uint8_t keycode, int16_t x,
                               int16_t y) {
  UtObject *object = ut_object_new(sizeof(UtX11KeyPress), &object_interface);
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  self->window = window;
  self->keycode = keycode;
  self->x = x;
  self->y = y;
  return object;
}

uint32_t ut_x11_key_press_get_window(UtObject *object) {
  assert(ut_object_is_x11_key_press(object));
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  return self->window;
}

uint8_t ut_x11_key_press_get_keycode(UtObject *object) {
  assert(ut_object_is_x11_key_press(object));
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  return self->keycode;
}

int16_t ut_x11_key_press_get_x(UtObject *object) {
  assert(ut_object_is_x11_key_press(object));
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  return self->x;
}

int16_t ut_x11_key_press_get_y(UtObject *object) {
  assert(ut_object_is_x11_key_press(object));
  UtX11KeyPress *self = (UtX11KeyPress *)object;
  return self->y;
}

bool ut_object_is_x11_key_press(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
