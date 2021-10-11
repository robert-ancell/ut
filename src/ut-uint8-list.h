#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const uint8_t *(*get_data)(UtObject *object);
} UtUint8ListFunctions;

extern int ut_uint8_list_id;

const uint8_t *ut_uint8_list_get_data(UtObject *object);
