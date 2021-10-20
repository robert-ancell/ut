#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const uint8_t *(*get_data)(UtObject *object);
  const size_t (*get_length)(UtObject *object);
  uint8_t *(*take_data)(UtObject *object);
} UtUint8ListFunctions;

extern int ut_uint8_list_id;

const uint8_t *ut_uint8_list_get_data(UtObject *object);

uint8_t *ut_uint8_list_take_data(UtObject *object);

bool ut_object_implements_uint8_list(UtObject *object);
