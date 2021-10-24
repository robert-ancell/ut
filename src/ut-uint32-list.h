#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint32_t (*get_element)(UtObject *object, size_t index);
} UtUint32ListInterface;

extern int ut_uint32_list_id;

uint32_t ut_uint32_list_get_element(UtObject *object, size_t index);

bool ut_object_implements_uint32_list(UtObject *object);
