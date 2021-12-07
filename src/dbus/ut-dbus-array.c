#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cstring.h"
#include "ut-dbus-array.h"
#include "ut-list.h"
#include "ut-object-list.h"

typedef struct {
  UtObject object;
  char *value_signature;
  UtObject *values;
} UtDBusArray;

static void ut_dbus_array_init(UtObject *object) {
  UtDBusArray *self = (UtDBusArray *)object;
  self->value_signature = NULL;
  self->values = ut_list_new();
}

static char *ut_dbus_array_to_string(UtObject *object) {
  UtDBusArray *self = (UtDBusArray *)object;
  ut_cstring_ref values_string = ut_object_to_string(self->values);
  return ut_cstring_new_printf("<UtDBusArray>(\"%s\", %s)",
                               self->value_signature, values_string);
}

static void ut_dbus_array_cleanup(UtObject *object) {
  UtDBusArray *self = (UtDBusArray *)object;
  free(self->value_signature);
  ut_object_unref(self->values);
}

static size_t ut_dbus_array_get_length(UtObject *object) {
  UtDBusArray *self = (UtDBusArray *)object;
  return ut_list_get_length(self->values);
}

static UtObject *ut_dbus_array_get_element(UtObject *object, size_t index) {
  UtDBusArray *self = (UtDBusArray *)object;
  return ut_object_list_get_element(self->values, index);
}

static UtObject *ut_dbus_array_get_element_ref(UtObject *object, size_t index) {
  UtDBusArray *self = (UtDBusArray *)object;
  return ut_list_get_element(self->values, index);
}

static UtObject *ut_dbus_array_copy(UtObject *object) {
  UtDBusArray *self = (UtDBusArray *)object;
  UtObject *copy = ut_dbus_array_new(self->value_signature);
  ut_list_append_list(copy, self->values);
  return copy;
}

static void ut_dbus_array_insert(UtObject *object, size_t index,
                                 UtObject *item) {
  UtDBusArray *self = (UtDBusArray *)object;
  ut_list_insert(self->values, index, item);
}

static void ut_dbus_array_remove(UtObject *object, size_t index, size_t count) {
  UtDBusArray *self = (UtDBusArray *)object;
  ut_list_remove(self->values, index, count);
}

static void ut_dbus_array_resize(UtObject *object, size_t length) {
  UtDBusArray *self = (UtDBusArray *)object;
  ut_list_resize(self->values, length);
}

static UtObjectListInterface object_list_interface = {
    .get_element = ut_dbus_array_get_element};

static UtListInterface list_interface = {.is_mutable = true,
                                         .get_length = ut_dbus_array_get_length,
                                         .get_element =
                                             ut_dbus_array_get_element_ref,
                                         .copy = ut_dbus_array_copy,
                                         .insert = ut_dbus_array_insert,
                                         .remove = ut_dbus_array_remove,
                                         .resize = ut_dbus_array_resize};

static UtObjectInterface object_interface = {
    .type_name = "UtDBusArray",
    .init = ut_dbus_array_init,
    .to_string = ut_dbus_array_to_string,
    .cleanup = ut_dbus_array_cleanup,
    .interfaces = {{&ut_object_list_id, &object_list_interface},
                   {&ut_list_id, &list_interface},
                   {NULL, NULL}}};

UtObject *ut_dbus_array_new(const char *value_signature) {
  UtObject *object = ut_object_new(sizeof(UtDBusArray), &object_interface);
  UtDBusArray *self = (UtDBusArray *)object;
  self->value_signature = strdup(value_signature);
  return object;
}

const char *ut_dbus_array_get_value_signature(UtObject *object) {
  assert(ut_object_is_dbus_array(object));
  UtDBusArray *self = (UtDBusArray *)object;
  return self->value_signature;
}

bool ut_object_is_dbus_array(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
