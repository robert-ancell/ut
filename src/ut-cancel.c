#include "ut-cancel.h"
#include "ut-object-private.h"

typedef struct {
  UtObject object;
  bool is_active;
} UtCancel;

static const char *ut_cancel_get_type_name() { return "Cancel"; }

static void ut_cancel_init(UtObject *object) {
  UtCancel *self = (UtCancel *)object;
  self->is_active = false;
}

static void ut_cancel_cleanup(UtObject *object) {}

static UtObjectFunctions object_functions = {.get_type_name =
                                                 ut_cancel_get_type_name,
                                             .init = ut_cancel_init,
                                             .cleanup = ut_cancel_cleanup};

UtObject *ut_cancel_new() {
  return ut_object_new(sizeof(UtCancel), &object_functions);
}

void ut_cancel_activate(UtObject *object) {
  UtCancel *self = (UtCancel *)object;
  self->is_active = true;
}

bool ut_cancel_is_active(UtObject *object) {
  UtCancel *self = (UtCancel *)object;
  return self->is_active;
}

bool ut_object_is_cancel(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
