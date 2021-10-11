#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  UtObject *(*get_element)(UtObject *object, size_t index);
} UtObjectListFunctions;

extern int ut_object_list_id;

UtObject *ut_object_list_get_element(UtObject *object, size_t index);
