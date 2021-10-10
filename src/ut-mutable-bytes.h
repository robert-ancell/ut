#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_bytes_new();

void ut_mutable_bytes_append(UtObject *object, uint8_t data);

void ut_mutable_bytes_append_block(UtObject *object, const uint8_t *data,
                                   size_t data_length);

void ut_mutable_bytes_insert(UtObject *object, size_t index, uint8_t data);

void ut_mutable_bytes_insert_block(UtObject *object, size_t index,
                                   const uint8_t *data, size_t data_length);

const uint8_t *ut_mutable_bytes_get_data(UtObject *object);

bool ut_object_is_mutable_bytes(UtObject *object);
