#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint16_t (*get_element)(UtObject *object, size_t index);
  uint16_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, uint16_t item);
} UtUint16ListInterface;

extern int ut_uint16_list_id;

UtObject *ut_uint16_list_new();

UtObject *ut_uint16_list_new_with_data(size_t length, ...);

uint16_t ut_uint16_list_get_element(UtObject *object, size_t index);

void ut_uint16_list_append(UtObject *object, uint16_t item);

void ut_uint16_list_prepend(UtObject *object, uint16_t item);

void ut_uint16_list_insert(UtObject *object, size_t index, uint16_t item);

bool ut_object_implements_uint16_list(UtObject *object);
