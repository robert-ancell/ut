#include "ut-x11-length-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
} UtX11LengthError;

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11LengthError",
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_length_error_new() {
  return ut_object_new(sizeof(UtX11LengthError), &object_interface);
}

bool ut_object_is_x11_length_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
