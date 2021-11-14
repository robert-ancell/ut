#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint16_t (*get_element)(UtObject *object, size_t index);
  uint16_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const uint16_t *data,
                 size_t data_length);
} UtUint16ListInterface;

extern int ut_uint16_list_id;

UtObject *ut_uint16_list_new();

UtObject *ut_uint16_list_new_with_data(size_t length, ...);

uint16_t ut_uint16_list_get_element(UtObject *object, size_t index);

void ut_uint16_list_append(UtObject *object, uint16_t item);

void ut_uint16_list_append_block(UtObject *object, const uint16_t *data,
                                 size_t data_length);

void ut_uint16_list_prepend(UtObject *object, uint16_t item);

void ut_uint16_list_prepend_block(UtObject *object, const uint16_t *data,
                                  size_t data_length);

void ut_uint16_list_insert(UtObject *object, size_t index, uint16_t item);

void ut_uint16_list_insert_block(UtObject *object, size_t index,
                                 const uint16_t *data, size_t data_length);

bool ut_object_implements_uint16_list(UtObject *object);
