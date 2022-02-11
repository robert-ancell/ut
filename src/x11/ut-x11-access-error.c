#include <assert.h>

#include "ut-x11-access-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
} UtX11AccessError;

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11AccessError",
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}, {NULL, NULL}}};

UtObject *ut_x11_access_error_new() {
  return ut_object_new(sizeof(UtX11AccessError), &object_interface);
}

bool ut_object_is_x11_access_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
