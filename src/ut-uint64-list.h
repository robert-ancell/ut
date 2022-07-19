#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint64_t (*get_element)(UtObject *object, size_t index);
  uint64_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const uint64_t *data,
                 size_t data_length);
} UtUint64ListInterface;

extern int ut_uint64_list_id;

UtObject *ut_uint64_list_new();

UtObject *ut_uint64_list_new_with_elements(size_t length, ...);

uint64_t ut_uint64_list_get_element(UtObject *object, size_t index);

uint64_t *ut_uint64_list_take_data(UtObject *object);

void ut_uint64_list_append(UtObject *object, uint64_t item);

void ut_uint64_list_append_block(UtObject *object, const uint64_t *data,
                                 size_t data_length);

void ut_uint64_list_prepend(UtObject *object, uint64_t item);

void ut_uint64_list_prepend_block(UtObject *object, const uint64_t *data,
                                  size_t data_length);

void ut_uint64_list_insert(UtObject *object, size_t index, uint64_t item);

void ut_uint64_list_insert_block(UtObject *object, size_t index,
                                 const uint64_t *data, size_t data_length);

bool ut_object_implements_uint64_list(UtObject *object);
