#include <assert.h>

#include "ut-cstring.h"
#include "ut-x11-event.h"
#include "ut-x11-unknown-generic-event.h"

typedef struct {
  UtObject object;
  uint8_t major_opcode;
  uint16_t code;
} UtX11UnknownGenericEvent;

static void ut_x11_unknown_generic_event_init(UtObject *object) {
  UtX11UnknownGenericEvent *self = (UtX11UnknownGenericEvent *)object;
  self->major_opcode = 0;
  self->code = 0;
}

static char *ut_x11_unknown_generic_event_to_string(UtObject *object) {
  UtX11UnknownGenericEvent *self = (UtX11UnknownGenericEvent *)object;
  return ut_cstring_new_printf("<UtX11UnknownGenericEvent>(%d, %d)",
                               self->major_opcode, self->code);
}

static UtX11EventInterface x11_event_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11UnknownGenericEvent",
    .init = ut_x11_unknown_generic_event_init,
    .to_string = ut_x11_unknown_generic_event_to_string,
    .interfaces = {{&ut_x11_event_id, &x11_event_interface}}};

UtObject *ut_x11_unknown_generic_event_new(uint8_t major_opcode,
                                           uint16_t code) {
  UtObject *object =
      ut_object_new(sizeof(UtX11UnknownGenericEvent), &object_interface);
  UtX11UnknownGenericEvent *self = (UtX11UnknownGenericEvent *)object;
  self->major_opcode = major_opcode;
  self->code = code;
  return object;
}

uint8_t ut_x11_unknown_generic_event_get_major_opcode(UtObject *object) {
  assert(ut_object_is_x11_unknown_generic_event(object));
  UtX11UnknownGenericEvent *self = (UtX11UnknownGenericEvent *)object;
  return self->major_opcode;
}

uint16_t ut_x11_unknown_generic_event_get_code(UtObject *object) {
  assert(ut_object_is_x11_unknown_generic_event(object));
  UtX11UnknownGenericEvent *self = (UtX11UnknownGenericEvent *)object;
  return self->code;
}

bool ut_object_is_x11_unknown_generic_event(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
