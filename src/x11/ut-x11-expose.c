#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-event.h"
#include "ut-x11-expose.h"

typedef struct {
  UtObject object;
  uint32_t window;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
} UtX11Expose;

static void ut_x11_expose_init(UtObject *object) {
  UtX11Expose *self = (UtX11Expose *)object;
  self->window = 0;
  self->x = 0;
  self->y = 0;
  self->width = 0;
  self->height = 0;
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11Expose",
    .init = ut_x11_expose_init,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_expose_new(uint32_t window, uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height) {
  UtObject *object = ut_object_new(sizeof(UtX11Expose), &object_interface);
  UtX11Expose *self = (UtX11Expose *)object;
  self->window = window;
  self->x = x;
  self->y = y;
  self->width = width;
  self->height = height;
  return object;
}

uint32_t ut_x11_expose_get_window(UtObject *object) {
  assert(ut_object_is_x11_expose(object));
  UtX11Expose *self = (UtX11Expose *)object;
  return self->window;
}

uint16_t ut_x11_expose_get_x(UtObject *object) {
  assert(ut_object_is_x11_expose(object));
  UtX11Expose *self = (UtX11Expose *)object;
  return self->x;
}

uint16_t ut_x11_expose_get_y(UtObject *object) {
  assert(ut_object_is_x11_expose(object));
  UtX11Expose *self = (UtX11Expose *)object;
  return self->y;
}

uint16_t ut_x11_expose_get_width(UtObject *object) {
  assert(ut_object_is_x11_expose(object));
  UtX11Expose *self = (UtX11Expose *)object;
  return self->width;
}

uint16_t ut_x11_expose_get_height(UtObject *object) {
  assert(ut_object_is_x11_expose(object));
  UtX11Expose *self = (UtX11Expose *)object;
  return self->height;
}

bool ut_object_is_x11_expose(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
