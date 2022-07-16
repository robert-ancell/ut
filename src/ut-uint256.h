#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_uint256_new(uint64_t value);

UtObject *ut_uint256_new_from_data(
    uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3, uint8_t v4, uint8_t v5,
    uint8_t v6, uint8_t v7, uint8_t v8, uint8_t v9, uint8_t v10, uint8_t v11,
    uint8_t v12, uint8_t v13, uint8_t v14, uint8_t v15, uint8_t v16,
    uint8_t v17, uint8_t v18, uint8_t v19, uint8_t v20, uint8_t v21,
    uint8_t v22, uint8_t v23, uint8_t v24, uint8_t v25, uint8_t v26,
    uint8_t v27, uint8_t v28, uint8_t v29, uint8_t v30, uint8_t v31);

UtObject *ut_uint256_new_from_uint8_list(UtObject *list);

UtObject *ut_uint256_copy(UtObject *object);

uint8_t ut_uint256_get_bit(UtObject *object, size_t n);

bool ut_uint256_is_uint64(UtObject *object);

uint64_t ut_uint256_to_uint64(UtObject *object);

UtObject *ut_uint256_to_uint8_list(UtObject *object);

void ut_uint256_set(UtObject *object, UtObject *value);

void ut_uint256_and(UtObject *object, UtObject *value);

void ut_uint256_or(UtObject *object, UtObject *value);

void ut_uint256_xor(UtObject *object, UtObject *value);

void ut_uint256_add(UtObject *object, UtObject *a, UtObject *b);

void ut_uint256_sub(UtObject *object, UtObject *a, UtObject *b);

void ut_uint256_mul(UtObject *object, UtObject *a, UtObject *b);

/// Swap the values of [object] and [a] if [swap] is true.
void ut_uint256_cswap(UtObject *object, bool swap, UtObject *v);

bool ut_object_is_uint256(UtObject *object);
