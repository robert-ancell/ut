#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint16_t (*get_element)(UtObject *object, size_t index);
} UtUint16ListInterface;

extern int ut_uint16_list_id;

uint16_t ut_uint16_list_get_element(UtObject *object, size_t index);

bool ut_object_implements_uint32_list(UtObject *object);
