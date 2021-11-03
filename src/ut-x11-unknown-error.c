#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-error.h"
#include "ut-x11-unknown-error.h"

typedef struct {
  UtObject object;
  uint8_t code;
  uint8_t major_opcode;
  uint16_t minor_opcode;
} UtX11UnknownError;

static void ut_x11_unknown_error_init(UtObject *object) {
  UtX11UnknownError *self = (UtX11UnknownError *)object;
  self->code = 0;
  self->major_opcode = 0;
  self->minor_opcode = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11UnknownError",
    .init = ut_x11_unknown_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_unknown_error_new(uint8_t code, uint8_t major_opcode,
                                   uint16_t minor_opcode) {
  UtObject *object =
      ut_object_new(sizeof(UtX11UnknownError), &object_interface);
  UtX11UnknownError *self = (UtX11UnknownError *)object;
  self->code = code;
  self->major_opcode = major_opcode;
  self->minor_opcode = minor_opcode;
  return object;
}

uint32_t ut_x11_unknown_error_get_code(UtObject *object) {
  assert(ut_object_is_x11_unknown_error(object));
  UtX11UnknownError *self = (UtX11UnknownError *)object;
  return self->code;
}

uint8_t ut_x11_unknown_error_get_major_opcode(UtObject *object) {
  assert(ut_object_is_x11_unknown_error(object));
  UtX11UnknownError *self = (UtX11UnknownError *)object;
  return self->major_opcode;
}

uint16_t ut_x11_unknown_error_get_minor_opcode(UtObject *object) {
  assert(ut_object_is_x11_unknown_error(object));
  UtX11UnknownError *self = (UtX11UnknownError *)object;
  return self->minor_opcode;
}

bool ut_object_is_x11_unknown_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
