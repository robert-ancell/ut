#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cstring.h"
#include "ut-dbus-error.h"
#include "ut-error.h"
#include "ut-list.h"
#include "ut-object-list.h"
#include "ut-string.h"

typedef struct {
  UtObject object;
  char *error_name;
  UtObject *args;
} UtDBusError;

static void ut_dbus_error_init(UtObject *object) {
  UtDBusError *self = (UtDBusError *)object;
  self->error_name = NULL;
  self->args = NULL;
}

static char *ut_dbus_error_to_string(UtObject *object) {
  UtDBusError *self = (UtDBusError *)object;
  return ut_cstring_new_printf("<UtDBusError>(%s)", self->error_name);
}

static void ut_dbus_error_cleanup(UtObject *object) {
  UtDBusError *self = (UtDBusError *)object;
  free(self->error_name);
  ut_object_unref(self->args);
}

static char *ut_dbus_error_dup_description(UtObject *object) {
  UtDBusError *self = (UtDBusError *)object;
  if (ut_list_get_length(self->args) > 0) {
    UtObject *arg0 = ut_object_list_get_element(self->args, 0);
    if (ut_object_implements_string(arg0)) {
      return ut_cstring_new_printf("%s: %s", self->error_name,
                                   ut_string_get_text(arg0));
    }
  }
  return ut_cstring_new_printf("%s", self->error_name);
}

static UtErrorInterface error_interface = {.get_description =
                                               ut_dbus_error_dup_description};

static UtObjectInterface object_interface = {
    .type_name = "UtDBusError",
    .init = ut_dbus_error_init,
    .to_string = ut_dbus_error_to_string,
    .cleanup = ut_dbus_error_cleanup,
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_dbus_error_new(const char *error_name, UtObject *args) {
  UtObject *object = ut_object_new(sizeof(UtDBusError), &object_interface);
  UtDBusError *self = (UtDBusError *)object;
  self->error_name = strdup(error_name);
  self->args = ut_object_ref(args);
  return object;
}

const char *ut_dbus_error_get_error_name(UtObject *object) {
  assert(ut_object_is_dbus_error(object));
  UtDBusError *self = (UtDBusError *)object;
  return self->error_name;
}

UtObject *ut_dbus_error_get_args(UtObject *object) {
  assert(ut_object_is_dbus_error(object));
  UtDBusError *self = (UtDBusError *)object;
  return self->args;
}

bool ut_object_is_dbus_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
