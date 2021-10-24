#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint8_t (*get_element)(UtObject *object, size_t index);
  uint8_t *(*take_data)(UtObject *object);
} UtUint8ListInterface;

extern int ut_uint8_list_id;

UtObject *ut_uint8_list_new();

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index);

uint8_t *ut_uint8_list_take_data(UtObject *object);

bool ut_object_implements_uint8_list(UtObject *object);
