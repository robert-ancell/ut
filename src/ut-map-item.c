#include <assert.h>

#include "ut-map-item.h"
#include "ut-object-private.h"

int ut_map_item_id = 0;

UtObject *ut_map_item_get_key(UtObject *object) {
  UtMapItemFunctions *map_item_functions =
      ut_object_get_interface(object, &ut_map_item_id);
  assert(map_item_functions != NULL);
  return map_item_functions->get_key(object);
}

UtObject *ut_map_item_get_value(UtObject *object) {
  UtMapItemFunctions *map_item_functions =
      ut_object_get_interface(object, &ut_map_item_id);
  assert(map_item_functions != NULL);
  return map_item_functions->get_value(object);
}

bool ut_object_implements_map_item(UtObject *object) {
  return ut_object_get_interface(object, &ut_map_item_id) != NULL;
}
