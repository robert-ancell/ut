#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const uint32_t *(*get_data)(UtObject *object);
} UtUint32ListFunctions;

extern int ut_uint32_list_id;

const uint32_t *ut_uint32_list_get_data(UtObject *object);

bool ut_object_implements_uint32_list(UtObject *object);
