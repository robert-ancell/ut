#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  uint8_t (*get_element)(UtObject *object, size_t index);
  uint8_t *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const uint8_t *data,
                 size_t data_length);
  void (*append)(UtObject *object, const uint8_t *data, size_t data_length);
} UtUint8ListInterface;

extern int ut_uint8_list_id;

UtObject *ut_uint8_list_new();

UtObject *ut_uint8_list_new_with_data(size_t length, ...);

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index);

uint8_t *ut_uint8_list_take_data(UtObject *object);

void ut_uint8_list_append(UtObject *object, uint8_t value);

void ut_uint8_list_append_block(UtObject *object, const uint8_t *data,
                                size_t data_length);

void ut_uint8_list_append_int8(UtObject *object, int8_t value);

void ut_uint8_list_append_uint16_le(UtObject *object, uint16_t value);

void ut_uint8_list_append_uint16_be(UtObject *object, uint16_t value);

void ut_uint8_list_append_int16_le(UtObject *object, int16_t value);

void ut_uint8_list_append_int16_be(UtObject *object, int16_t value);

void ut_uint8_list_append_uint32_le(UtObject *object, uint32_t value);

void ut_uint8_list_append_uint32_be(UtObject *object, uint32_t value);

void ut_uint8_list_append_int32_le(UtObject *object, int32_t value);

void ut_uint8_list_append_int32_be(UtObject *object, int32_t value);

void ut_uint8_list_prepend(UtObject *object, uint8_t value);

void ut_uint8_list_prepend_block(UtObject *object, const uint8_t *data,
                                 size_t data_length);

void ut_uint8_list_insert(UtObject *object, size_t index, uint8_t value);

void ut_uint8_list_insert_block(UtObject *object, size_t index,
                                const uint8_t *data, size_t data_length);

bool ut_object_implements_uint8_list(UtObject *object);
