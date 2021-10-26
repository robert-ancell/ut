#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint8_t (*get_element)(UtObject *object, size_t index);
  uint8_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, uint8_t item);
} UtUint8ListInterface;

extern int ut_uint8_list_id;

UtObject *ut_uint8_list_new();

UtObject *ut_uint8_list_new_with_data(size_t length, ...);

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index);

uint8_t *ut_uint8_list_take_data(UtObject *object);

void ut_uint8_list_append(UtObject *object, uint8_t item);

void ut_uint8_list_prepend(UtObject *object, uint8_t item);

void ut_uint8_list_insert(UtObject *object, size_t index, uint8_t item);

bool ut_object_implements_uint8_list(UtObject *object);
