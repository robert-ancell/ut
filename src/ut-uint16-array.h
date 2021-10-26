#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint16_array_new();

UtObject *ut_uint16_array_new_with_data(size_t length, ...);

UtObject *ut_uint16_array_new_with_va_data(size_t length, va_list ap);

void ut_uint16_array_append(UtObject *object, uint16_t data);

void ut_uint16_array_append_block(UtObject *object, const uint16_t *data,
                                  size_t data_length);

void ut_uint16_array_insert(UtObject *object, size_t index, uint16_t data);

void ut_uint16_array_insert_block(UtObject *object, size_t index,
                                  const uint16_t *data, size_t data_length);

uint16_t *ut_uint16_array_get_data(UtObject *object);

bool ut_object_is_uint16_array(UtObject *object);
