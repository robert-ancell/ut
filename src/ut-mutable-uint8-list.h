#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_uint8_list_new();

void ut_mutable_uint8_list_append(UtObject *object, uint8_t data);

void ut_mutable_uint8_list_append_block(UtObject *object, const uint8_t *data,
                                        size_t data_length);

void ut_mutable_uint8_list_insert(UtObject *object, size_t index, uint8_t data);

void ut_mutable_uint8_list_insert_block(UtObject *object, size_t index,
                                        const uint8_t *data,
                                        size_t data_length);

uint8_t *ut_mutable_uint8_list_get_data(UtObject *object);

bool ut_object_is_mutable_uint8_list(UtObject *object);
