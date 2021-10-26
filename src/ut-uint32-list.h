#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint32_t (*get_element)(UtObject *object, size_t index);
  uint32_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, uint32_t item);
} UtUint32ListInterface;

extern int ut_uint32_list_id;

UtObject *ut_uint32_list_new();

UtObject *ut_uint32_list_new_with_data(size_t length, ...);

uint32_t ut_uint32_list_get_element(UtObject *object, size_t index);

uint32_t *ut_uint32_list_take_data(UtObject *object);

void ut_uint32_list_append(UtObject *object, uint32_t item);

void ut_uint32_list_prepend(UtObject *object, uint32_t item);

void ut_uint32_list_insert(UtObject *object, size_t index, uint32_t item);

bool ut_object_implements_uint32_list(UtObject *object);
