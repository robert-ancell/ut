#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-dbus-dict.h"
#include "ut-map.h"

typedef struct {
  UtObject object;
  char *key_signature;
  char *value_signature;
  UtObject *data;
} UtDBusDict;

static void ut_dbus_dict_init(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  self->key_signature = NULL;
  self->value_signature = NULL;
  self->data = ut_map_new();
}

static void ut_dbus_dict_cleanup(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  free(self->key_signature);
  free(self->value_signature);
  ut_object_unref(self->data);
}

static UtObjectInterface object_interface = {.type_name = "UtDBusDict",
                                             .init = ut_dbus_dict_init,
                                             .cleanup = ut_dbus_dict_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_dbus_dict_new(const char *key_signature,
                           const char *value_signature) {
  UtObject *object = ut_object_new(sizeof(UtDBusDict), &object_interface);
  UtDBusDict *self = (UtDBusDict *)object;
  self->key_signature = strdup(key_signature);
  self->value_signature = strdup(value_signature);
  return object;
}

const char *ut_dbus_dict_get_key_signature(UtObject *object) {
  assert(ut_object_is_dbus_dict(object));
  UtDBusDict *self = (UtDBusDict *)object;
  return self->key_signature;
}

const char *ut_dbus_dict_get_value_signature(UtObject *object) {
  assert(ut_object_is_dbus_dict(object));
  UtDBusDict *self = (UtDBusDict *)object;
  return self->value_signature;
}

bool ut_object_is_dbus_dict(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
