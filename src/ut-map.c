#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-hash-map.h"
#include "ut-list.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-mutable-string.h"
#include "ut-object-private.h"
#include "ut-string.h"

int ut_map_id = 0;

UtObject *ut_map_new() { return ut_hash_map_new(); }

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
  UtObject *key_ = ut_string_new(key);
  ut_map_insert(object, key_, value);
  ut_object_unref(key_);
}

void ut_map_insert_string_take(UtObject *object, const char *key,
                               UtObject *value) {
  ut_map_insert_take(object, ut_string_new(key), value);
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

char *ut_map_to_string(UtObject *object) {
  UtObject *string = ut_mutable_string_new("{");
  UtObject *items = ut_map_get_items(object);
  for (size_t i = 0; i < ut_list_get_length(items); i++) {
    UtObject *item = ut_list_get_element(items, i);

    if (i != 0) {
      ut_mutable_string_append(string, ", ");
    }

    char *key_string = ut_object_to_string(ut_map_item_get_key(item));
    ut_mutable_string_append(string, key_string);
    free(key_string);

    ut_mutable_string_append(string, ": ");

    char *value_string = ut_object_to_string(ut_map_item_get_value(item));
    ut_mutable_string_append(string, value_string);
    free(value_string);

    ut_object_unref(item);
  }
  ut_object_unref(items);
  ut_mutable_string_append(string, "}");

  char *result = strdup(ut_string_get_text(string));
  ut_object_unref(string);
  return result;
}

bool ut_object_implements_map(UtObject *object) {
  return ut_object_get_interface(object, &ut_map_id) != NULL;
}
