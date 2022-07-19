#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  int32_t (*get_element)(UtObject *object, size_t index);
  int32_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const int32_t *data,
                 size_t data_length);
} UtInt32ListInterface;

extern int ut_int32_list_id;

UtObject *ut_int32_list_new();

UtObject *ut_int32_list_new_with_elements(size_t length, ...);

int32_t ut_int32_list_get_element(UtObject *object, size_t index);

int32_t *ut_int32_list_take_data(UtObject *object);

void ut_int32_list_append(UtObject *object, int32_t item);

void ut_int32_list_append_block(UtObject *object, const int32_t *data,
                                size_t data_length);

void ut_int32_list_prepend(UtObject *object, int32_t item);

void ut_int32_list_prepend_block(UtObject *object, const int32_t *data,
                                 size_t data_length);

void ut_int32_list_insert(UtObject *object, size_t index, int32_t item);

void ut_int32_list_insert_block(UtObject *object, size_t index,
                                const int32_t *data, size_t data_length);

bool ut_object_implements_int32_list(UtObject *object);
