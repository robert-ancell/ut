#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  UtObject *(*get_element)(UtObject *object, size_t index);
} UtObjectListInterface;

extern int ut_object_list_id;

UtObject *ut_object_list_new();

UtObject *ut_object_list_get_element(UtObject *object, size_t index);

bool ut_object_implements_object_list(UtObject *object);
