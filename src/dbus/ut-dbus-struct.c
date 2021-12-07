#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cstring.h"
#include "ut-dbus-struct.h"
#include "ut-list.h"
#include "ut-object-list.h"

typedef struct {
  UtObject object;
  UtObject *values;
} UtDBusStruct;

static void ut_dbus_struct_init(UtObject *object) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  self->values = ut_object_list_new();
}

static char *ut_dbus_struct_to_string(UtObject *object) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  ut_cstring_ref values_string = ut_object_to_string(self->values);
  return ut_cstring_new_printf("<UtDBusStruct>(%s)", values_string);
}

static void ut_dbus_struct_cleanup(UtObject *object) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  ut_object_unref(self->values);
}

static size_t ut_dbus_struct_get_length(UtObject *object) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  return ut_list_get_length(self->values);
}

static UtObject *ut_dbus_struct_get_element(UtObject *object, size_t index) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  return ut_object_list_get_element(self->values, index);
}

static UtObject *ut_dbus_struct_get_element_ref(UtObject *object,
                                                size_t index) {
  UtDBusStruct *self = (UtDBusStruct *)object;
  return ut_list_get_element(self->values, index);
}

static UtObjectListInterface object_list_interface = {
    .get_element = ut_dbus_struct_get_element};

static UtListInterface list_interface = {
    .is_mutable = false,
    .get_length = ut_dbus_struct_get_length,
    .get_element = ut_dbus_struct_get_element_ref};

static UtObjectInterface object_interface = {
    .type_name = "UtDBusStruct",
    .init = ut_dbus_struct_init,
    .to_string = ut_dbus_struct_to_string,
    .cleanup = ut_dbus_struct_cleanup,
    .interfaces = {{&ut_object_list_id, &object_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_dbus_struct_new(UtObject *value0, ...) {
  UtObject *object = ut_object_new(sizeof(UtDBusStruct), &object_interface);
  UtDBusStruct *self = (UtDBusStruct *)object;
  assert(value0 != NULL);
  ut_list_append(self->values, value0);
  va_list ap;
  va_start(ap, value0);
  UtObject *value;
  while ((value = va_arg(ap, UtObject *)) != NULL) {
    ut_list_append(self->values, value);
  }
  va_end(ap);
  return object;
}

UtObject *ut_dbus_struct_new_take(UtObject *value0, ...) {
  UtObject *object = ut_object_new(sizeof(UtDBusStruct), &object_interface);
  UtDBusStruct *self = (UtDBusStruct *)object;
  assert(value0 != NULL);
  ut_list_append(self->values, ut_object_ref(value0));
  va_list ap;
  va_start(ap, value0);
  UtObject *value;
  while ((value = va_arg(ap, UtObject *)) != NULL) {
    ut_list_append(self->values, ut_object_ref(value));
  }
  va_end(ap);
  return object;
}

UtObject *ut_dbus_struct_new_from_list(UtObject *values) {
  UtObject *object = ut_object_new(sizeof(UtDBusStruct), &object_interface);
  UtDBusStruct *self = (UtDBusStruct *)object;
  ut_list_append_list(self->values, values);
  return object;
}

UtObject *ut_dbus_struct_get_value(UtObject *object, size_t index) {
  assert(ut_object_is_dbus_struct(object));
  UtDBusStruct *self = (UtDBusStruct *)object;
  return ut_object_list_get_element(self->values, index);
}

UtObject *ut_dbus_struct_get_values(UtObject *object) {
  assert(ut_object_is_dbus_struct(object));
  UtDBusStruct *self = (UtDBusStruct *)object;
  return self->values;
}

bool ut_object_is_dbus_struct(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
