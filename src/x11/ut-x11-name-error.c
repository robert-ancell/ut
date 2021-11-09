#include "ut-x11-name-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
} UtX11NameError;

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11NameError",
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}}};

UtObject *ut_x11_name_error_new() {
  return ut_object_new(sizeof(UtX11NameError), &object_interface);
}

bool ut_object_is_x11_name_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
