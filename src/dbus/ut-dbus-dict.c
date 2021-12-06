#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-dbus-dict.h"
#include "ut-map-item.h"
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

size_t ut_dbus_dict_get_length(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  return ut_map_get_length(self->data);
}

static void ut_dbus_dict_insert(UtObject *object, UtObject *key,
                                UtObject *value) {
  UtDBusDict *self = (UtDBusDict *)object;
  ut_map_insert(self->data, key, value);
}

static UtObject *ut_dbus_dict_lookup(UtObject *object, UtObject *key) {
  UtDBusDict *self = (UtDBusDict *)object;
  return ut_map_lookup(self->data, key);
}

static void ut_dbus_dict_remove(UtObject *object, UtObject *key) {
  UtDBusDict *self = (UtDBusDict *)object;
  ut_map_remove(self->data, key);
}

static UtObject *ut_dbus_dict_get_items(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  return ut_map_get_items(self->data);
}

static UtObject *ut_dbus_dict_get_keys(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  return ut_map_get_keys(self->data);
}

static UtObject *ut_dbus_dict_get_values(UtObject *object) {
  UtDBusDict *self = (UtDBusDict *)object;
  return ut_map_get_values(self->data);
}

static UtMapInterface map_interface = {.get_length = ut_dbus_dict_get_length,
                                       .insert = ut_dbus_dict_insert,
                                       .lookup = ut_dbus_dict_lookup,
                                       .remove = ut_dbus_dict_remove,
                                       .get_items = ut_dbus_dict_get_items,
                                       .get_keys = ut_dbus_dict_get_keys,
                                       .get_values = ut_dbus_dict_get_values};

static UtObjectInterface object_interface = {
    .type_name = "UtDBusDict",
    .init = ut_dbus_dict_init,
    .cleanup = ut_dbus_dict_cleanup,
    .interfaces = {{&ut_map_id, &map_interface}, {NULL, NULL}}};

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
