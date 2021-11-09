#include "ut-cancel.h"

typedef struct {
  UtObject object;
  bool is_active;
} UtCancel;

static void ut_cancel_init(UtObject *object) {
  UtCancel *self = (UtCancel *)object;
  self->is_active = false;
}

static UtObjectInterface object_interface = {.type_name = "UtCancel",
                                             .init = ut_cancel_init};

UtObject *ut_cancel_new() {
  return ut_object_new(sizeof(UtCancel), &object_interface);
}

void ut_cancel_activate(UtObject *object) {
  UtCancel *self = (UtCancel *)object;
  self->is_active = true;
}

bool ut_cancel_is_active(UtObject *object) {
  if (object == NULL) {
    return false;
  }
  UtCancel *self = (UtCancel *)object;
  return self->is_active;
}

bool ut_object_is_cancel(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
