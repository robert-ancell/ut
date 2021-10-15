#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  UtObject *(*get_key)(UtObject *object);
  UtObject *(*get_value)(UtObject *object);
} UtMapItemFunctions;

extern int ut_map_item_id;

UtObject *ut_map_item_get_key(UtObject *object);

UtObject *ut_map_item_get_value(UtObject *object);

bool ut_object_implements_map_item(UtObject *object);
