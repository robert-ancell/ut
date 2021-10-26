#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint64_array_new();

UtObject *ut_uint64_array_new_with_data(size_t length, ...);

void ut_uint64_array_append(UtObject *object, uint64_t data);

void ut_uint64_array_append_block(UtObject *object, const uint64_t *data,
                                  size_t data_length);

void ut_uint64_array_insert(UtObject *object, size_t index, uint64_t data);

void ut_uint64_array_insert_block(UtObject *object, size_t index,
                                  const uint64_t *data, size_t data_length);

uint64_t *ut_uint64_array_get_data(UtObject *object);

bool ut_object_is_uint64_array(UtObject *object);
