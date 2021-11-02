#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-unknown-event.h"

typedef struct {
  UtObject object;
  uint8_t code;
} UtX11UnknownEvent;

static void ut_x11_unknown_event_init(UtObject *object) {
  UtX11UnknownEvent *self = (UtX11UnknownEvent *)object;
  self->code = 0;
}

static UtObjectInterface object_interface = {.type_name = "UtX11UnknownEvent",
                                             .init = ut_x11_unknown_event_init,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_x11_unknown_event_new(uint8_t code) {
  UtObject *object =
      ut_object_new(sizeof(UtX11UnknownEvent), &object_interface);
  UtX11UnknownEvent *self = (UtX11UnknownEvent *)object;
  self->code = code;
  return object;
}

uint8_t ut_x11_unknown_event_get_code(UtObject *object) {
  assert(ut_object_is_x11_unknown_event(object));
  UtX11UnknownEvent *self = (UtX11UnknownEvent *)object;
  return self->code;
}

bool ut_object_is_x11_unknown_event(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
