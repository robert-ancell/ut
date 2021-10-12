#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_mutable_uint16_list_new();

void ut_mutable_uint16_list_append(UtObject *object, uint16_t data);

void ut_mutable_uint16_list_append_block(UtObject *object, const uint16_t *data,
                                         size_t data_length);

void ut_mutable_uint16_list_insert(UtObject *object, size_t index,
                                   uint16_t data);

void ut_mutable_uint16_list_insert_block(UtObject *object, size_t index,
                                         const uint16_t *data,
                                         size_t data_length);

uint16_t *ut_mutable_uint16_list_get_data(UtObject *object);

bool ut_object_is_mutable_uint16_list(UtObject *object);
