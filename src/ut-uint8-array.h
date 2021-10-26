#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint8_array_new();

UtObject *ut_uint8_array_new_with_data(size_t length, ...);

void ut_uint8_array_append(UtObject *object, uint8_t data);

void ut_uint8_array_append_block(UtObject *object, const uint8_t *data,
                                 size_t data_length);

void ut_uint8_array_insert(UtObject *object, size_t index, uint8_t data);

void ut_uint8_array_insert_block(UtObject *object, size_t index,
                                 const uint8_t *data, size_t data_length);

uint8_t *ut_uint8_array_get_data(UtObject *object);

bool ut_object_is_uint8_array(UtObject *object);
