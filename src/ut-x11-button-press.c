#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-button-press.h"

typedef struct {
  UtObject object;
  uint32_t window;
  uint8_t button;
  int16_t x;
  int16_t y;
} UtX11ButtonPress;

static void ut_x11_button_press_init(UtObject *object) {
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  self->window = 0;
  self->button = 0;
  self->x = 0;
  self->y = 0;
}

static UtObjectInterface object_interface = {.type_name = "UtX11ButtonPress",
                                             .init = ut_x11_button_press_init,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_button_press_new(uint32_t window, uint8_t button, int16_t x,
                                  int16_t y) {
  UtObject *object = ut_object_new(sizeof(UtX11ButtonPress), &object_interface);
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  self->window = window;
  self->button = button;
  self->x = x;
  self->y = y;
  return object;
}

uint32_t ut_x11_button_press_get_window(UtObject *object) {
  assert(ut_object_is_x11_button_press(object));
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  return self->window;
}

uint8_t ut_x11_button_press_get_button(UtObject *object) {
  assert(ut_object_is_x11_button_press(object));
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  return self->button;
}

int16_t ut_x11_button_press_get_x(UtObject *object) {
  assert(ut_object_is_x11_button_press(object));
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  return self->x;
}

int16_t ut_x11_button_press_get_y(UtObject *object) {
  assert(ut_object_is_x11_button_press(object));
  UtX11ButtonPress *self = (UtX11ButtonPress *)object;
  return self->y;
}

bool ut_object_is_x11_button_press(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
