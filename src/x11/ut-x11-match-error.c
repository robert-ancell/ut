#include "ut-x11-match-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
} UtX11MatchError;

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11MatchError",
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}, {NULL, NULL}}};

UtObject *ut_x11_match_error_new() {
  return ut_object_new(sizeof(UtX11MatchError), &object_interface);
}

bool ut_object_is_x11_match_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
