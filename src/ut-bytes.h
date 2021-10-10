#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const uint8_t *(*get_data)(UtObject *object);
} UtBytesFunctions;

extern int ut_bytes_id;

const uint8_t *ut_bytes_get_data(UtObject *object);
