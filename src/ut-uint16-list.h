#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const uint16_t *(*get_data)(UtObject *object);
} UtUint16ListFunctions;

extern int ut_uint16_list_id;

const uint16_t *ut_uint16_list_get_data(UtObject *object);

bool ut_object_implements_uint32_list(UtObject *object);
