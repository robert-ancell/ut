#include <assert.h>

#include "ut-object-private.h"
#include "ut-x11-error.h"
#include "ut-x11-id-choice-error.h"

typedef struct {
  UtObject object;
  uint32_t resource_id;
} UtX11IdChoiceError;

static void ut_x11_id_choice_error_init(UtObject *object) {
  UtX11IdChoiceError *self = (UtX11IdChoiceError *)object;
  self->resource_id = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11IdChoiceError",
    .init = ut_x11_id_choice_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_id_choice_error_new(uint32_t resource_id) {
  UtObject *object =
      ut_object_new(sizeof(UtX11IdChoiceError), &object_interface);
  UtX11IdChoiceError *self = (UtX11IdChoiceError *)object;
  self->resource_id = resource_id;
  return object;
}

uint32_t ut_x11_id_choice_error_get_resource_id(UtObject *object) {
  assert(ut_object_is_x11_id_choice_error(object));
  UtX11IdChoiceError *self = (UtX11IdChoiceError *)object;
  return self->resource_id;
}

bool ut_object_is_x11_id_choice_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
