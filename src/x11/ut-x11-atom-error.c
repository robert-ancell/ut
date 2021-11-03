#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-atom-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
  uint32_t atom;
} UtX11AtomError;

static void ut_x11_atom_error_init(UtObject *object) {
  UtX11AtomError *self = (UtX11AtomError *)object;
  self->atom = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11AtomError",
    .init = ut_x11_atom_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_atom_error_new(uint32_t atom) {
  UtObject *object = ut_object_new(sizeof(UtX11AtomError), &object_interface);
  UtX11AtomError *self = (UtX11AtomError *)object;
  self->atom = atom;
  return object;
}

uint32_t ut_x11_atom_error_get_atom(UtObject *object) {
  assert(ut_object_is_x11_atom_error(object));
  UtX11AtomError *self = (UtX11AtomError *)object;
  return self->atom;
}

bool ut_object_is_x11_atom_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
