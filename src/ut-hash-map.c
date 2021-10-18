#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-hash-map.h"
#include "ut-map-item.h"
#include "ut-map.h"
#include "ut-mutable-list.h"
#include "ut-object-array.h"
#include "ut-object-private.h"

typedef struct _UtHashMapItem UtHashMapItem;

typedef struct {
  UtObject object;
  UtHashMapItem *items;
  size_t length;
} UtHashMap;

struct _UtHashMapItem {
  UtObject object;
  UtObject *key;
  UtObject *value;
  UtHashMapItem *next;
};

static UtObject *ut_hash_map_item_get_key(UtObject *object) {
  UtHashMapItem *self = (UtHashMapItem *)object;
  return self->key;
}

static UtObject *ut_hash_map_item_get_value(UtObject *object) {
  UtHashMapItem *self = (UtHashMapItem *)object;
  return self->value;
}

static UtMapItemFunctions map_item_functions = {
    .get_key = ut_hash_map_item_get_key,
    .get_value = ut_hash_map_item_get_value};

static void ut_hash_map_item_init(UtObject *object) {
  UtHashMapItem *self = (UtHashMapItem *)object;
  self->key = NULL;
  self->value = NULL;
}

static void ut_hash_map_item_cleanup(UtObject *object) {
  UtHashMapItem *self = (UtHashMapItem *)object;
  ut_object_unref(self->key);
  self->key = NULL;
  ut_object_unref(self->value);
  self->value = NULL;
  self->next = NULL;
}

static UtObjectFunctions item_object_functions = {
    .type_name = "HashMapItem",
    .init = ut_hash_map_item_init,
    .cleanup = ut_hash_map_item_cleanup,
    .interfaces = {{&ut_map_item_id, &map_item_functions}, {NULL, NULL}}};

static UtHashMapItem *item_new(UtObject *key, UtObject *value) {
  UtHashMapItem *item = (UtHashMapItem *)ut_object_new(sizeof(UtHashMapItem),
                                                       &item_object_functions);
  item->key = ut_object_ref(key);
  item->value = ut_object_ref(value);
  return item;
}

static UtHashMapItem *lookup(UtHashMap *self, UtObject *key,
                             UtHashMapItem **prev_item) {
  UtHashMapItem *prev_item_ = NULL;
  for (UtHashMapItem *item = self->items; item != NULL; item = item->next) {
    if (ut_object_equal(item->key, key)) {
      if (prev_item != NULL) {
        *prev_item = prev_item_;
      }

      return item;
    }
    prev_item_ = item;
  }
  return NULL;
}

size_t ut_hash_map_get_length(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  return self->length;
}

static void ut_hash_map_insert(UtObject *object, UtObject *key,
                               UtObject *value) {
  UtHashMap *self = (UtHashMap *)object;

  UtHashMapItem *prev_item;
  UtHashMapItem *existing_item = lookup(self, key, &prev_item);

  UtHashMapItem *item = item_new(key, value);
  if (existing_item == NULL) {
    item->next = self->items;
    self->items = item;
  } else {
    item->next = existing_item->next;
    if (prev_item != NULL) {
      prev_item->next = item;
    } else {
      self->items = item;
    }

    existing_item->next = NULL;
    ut_object_unref((UtObject *)existing_item);
  }
}

static UtObject *ut_hash_map_lookup(UtObject *object, UtObject *key) {
  UtHashMap *self = (UtHashMap *)object;
  UtHashMapItem *item = lookup(self, key, NULL);
  return item != NULL ? ut_object_ref(item->value) : NULL;
}

static void ut_hash_map_remove(UtObject *object, UtObject *key) {
  UtHashMap *self = (UtHashMap *)object;
  UtHashMapItem *prev_item;
  UtHashMapItem *item = lookup(self, key, &prev_item);
  if (item == NULL) {
    return;
  }
  if (prev_item != NULL) {
    prev_item->next = item->next;
  } else {
    self->items = item->next;
  }

  item->next = NULL;
  ut_object_unref((UtObject *)item);
}

static UtObject *ut_hash_map_get_items(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  UtObject *items = ut_object_array_new();
  for (UtHashMapItem *item = self->items; item != NULL; item = item->next) {
    ut_mutable_list_append(items, (UtObject *)item);
  }
  return items;
}

static UtObject *ut_hash_map_get_keys(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  UtObject *keys = ut_object_array_new();
  for (UtHashMapItem *item = self->items; item != NULL; item = item->next) {
    ut_mutable_list_append(keys, item->key);
  }
  return keys;
}

static UtObject *ut_hash_map_get_values(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  UtObject *values = ut_object_array_new();
  for (UtHashMapItem *item = self->items; item != NULL; item = item->next) {
    ut_mutable_list_append(values, item->value);
  }
  return values;
}

static UtMapFunctions map_functions = {.get_length = ut_hash_map_get_length,
                                       .insert = ut_hash_map_insert,
                                       .lookup = ut_hash_map_lookup,
                                       .remove = ut_hash_map_remove,
                                       .get_items = ut_hash_map_get_items,
                                       .get_keys = ut_hash_map_get_keys,
                                       .get_values = ut_hash_map_get_values};

static void ut_hash_map_init(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  self->items = NULL;
  self->length = 0;
}

static void ut_hash_map_cleanup(UtObject *object) {
  UtHashMap *self = (UtHashMap *)object;
  UtHashMapItem *next_item;
  for (UtHashMapItem *item = self->items; item != NULL; item = next_item) {
    next_item = item->next;
    item->next = NULL;
    ut_object_unref((UtObject *)item);
  }
  self->items = NULL;
}

static UtObjectFunctions object_functions = {
    .type_name = "HashMap",
    .init = ut_hash_map_init,
    .to_string = ut_map_to_string,
    .cleanup = ut_hash_map_cleanup,
    .interfaces = {{&ut_map_id, &map_functions}, {NULL, NULL}}};

UtObject *ut_hash_map_new() {
  return ut_object_new(sizeof(UtHashMap), &object_functions);
}

bool ut_object_is_hash_map_string(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
