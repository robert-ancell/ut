#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint64_t (*get_element)(UtObject *object, size_t index);
} UtUint64ListInterface;

extern int ut_uint64_list_id;

uint64_t ut_uint64_list_get_element(UtObject *object, size_t index);

bool ut_object_implements_uint64_list(UtObject *object);
