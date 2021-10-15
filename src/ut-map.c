#include <assert.h>

#include "ut-immutable-string.h"
#include "ut-map.h"
#include "ut-object-private.h"

int ut_map_id = 0;

size_t ut_map_get_length(UtObject *object) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->get_length(object);
}

void ut_map_insert(UtObject *object, UtObject *key, UtObject *value) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->insert(object, key, value);
}

void ut_map_insert_take(UtObject *object, UtObject *key, UtObject *value) {
  ut_map_insert(object, key, value);
  ut_object_unref(key);
  ut_object_unref(value);
}

void ut_map_insert_string(UtObject *object, const char *key, UtObject *value) {
  UtObject *key_ = ut_immutable_string_new(key);
  ut_map_insert(object, key_, value);
  ut_object_unref(key_);
}

void ut_map_insert_string_take(UtObject *object, const char *key,
                               UtObject *value) {
  ut_map_insert_take(object, ut_immutable_string_new(key), value);
}

UtObject *ut_map_lookup(UtObject *object, UtObject *key) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->lookup(object, key);
}

void ut_map_remove(UtObject *object, UtObject *key) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  map_functions->remove(object, key);
}

UtObject *ut_map_get_items(UtObject *object) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->get_items(object);
}

UtObject *ut_map_get_keys(UtObject *object) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->get_keys(object);
}

UtObject *ut_map_get_values(UtObject *object) {
  UtMapFunctions *map_functions = ut_object_get_interface(object, &ut_map_id);
  assert(map_functions != NULL);
  return map_functions->get_values(object);
}

bool ut_object_implements_map(UtObject *object) {
  return ut_object_get_interface(object, &ut_map_id) != NULL;
}
