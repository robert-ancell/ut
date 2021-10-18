#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint32_array_new();

void ut_uint32_array_append(UtObject *object, uint32_t data);

void ut_uint32_array_append_block(UtObject *object, const uint32_t *data,
                                  size_t data_length);

void ut_uint32_array_insert(UtObject *object, size_t index, uint32_t data);

void ut_uint32_array_insert_block(UtObject *object, size_t index,
                                  const uint32_t *data, size_t data_length);

uint32_t *ut_uint32_array_get_data(UtObject *object);

bool ut_object_is_uint32_array(UtObject *object);
