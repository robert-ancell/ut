#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cstring.h"
#include "ut-hash-table.h"
#include "ut-list.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-object-private.h"
#include "ut-string.h"

int ut_map_id = 0;

UtObject *ut_map_new() { return ut_hash_table_new(); }

size_t ut_map_get_length(UtObject *object) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->get_length(object);
}

void ut_map_insert(UtObject *object, UtObject *key, UtObject *value) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->insert(object, key, value);
}

void ut_map_insert_take(UtObject *object, UtObject *key, UtObject *value) {
  ut_map_insert(object, key, value);
  ut_object_unref(key);
  ut_object_unref(value);
}

void ut_map_insert_string(UtObject *object, const char *key, UtObject *value) {
  UtObjectRef key_object = ut_string_new(key);
  ut_map_insert(object, key_object, value);
}

void ut_map_insert_string_take(UtObject *object, const char *key,
                               UtObject *value) {
  ut_map_insert_take(object, ut_string_new(key), value);
}

UtObject *ut_map_lookup(UtObject *object, UtObject *key) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->lookup(object, key);
}

UtObject *ut_map_lookup_string(UtObject *object, const char *key) {
  UtObjectRef string_key = ut_string_new_constant(key);
  return ut_map_lookup(object, string_key);
}

void ut_map_remove(UtObject *object, UtObject *key) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  map_interface->remove(object, key);
}

UtObject *ut_map_get_items(UtObject *object) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->get_items(object);
}

UtObject *ut_map_get_keys(UtObject *object) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->get_keys(object);
}

UtObject *ut_map_get_values(UtObject *object) {
  UtMapInterface *map_interface = ut_object_get_interface(object, &ut_map_id);
  assert(map_interface != NULL);
  return map_interface->get_values(object);
}

char *ut_map_to_string(UtObject *object) {
  UtObjectRef string = ut_string_new("{");
  UtObjectRef items = ut_map_get_items(object);
  for (size_t i = 0; i < ut_list_get_length(items); i++) {
    UtObjectRef item = ut_list_get_element(items, i);

    if (i != 0) {
      ut_string_append(string, ", ");
    }

    UtObjectRef key = ut_map_item_get_key(item);
    ut_cstring_ref key_string = ut_object_to_string(key);
    ut_string_append(string, key_string);

    ut_string_append(string, ": ");

    UtObjectRef value = ut_map_item_get_value(item);
    ut_cstring_ref value_string = ut_object_to_string(value);
    ut_string_append(string, value_string);
  }
  ut_string_append(string, "}");

  return ut_string_take_text(string);
}

bool ut_object_implements_map(UtObject *object) {
  return ut_object_get_interface(object, &ut_map_id) != NULL;
}
