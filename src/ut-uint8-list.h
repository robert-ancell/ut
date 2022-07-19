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

UtObject *ut_uint8_list_new_with_elements(size_t length, ...);

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index);

int8_t ut_uint8_list_get_int8(UtObject *object, size_t index);

uint16_t ut_uint8_list_get_uint16_le(UtObject *object, size_t index);

uint16_t ut_uint8_list_get_uint16_be(UtObject *object, size_t index);

int16_t ut_uint8_list_get_int16_le(UtObject *object, size_t index);

int16_t ut_uint8_list_get_int16_be(UtObject *object, size_t index);

uint32_t ut_uint8_list_get_uint32_le(UtObject *object, size_t index);

uint32_t ut_uint8_list_get_uint32_be(UtObject *object, size_t index);

int32_t ut_uint8_list_get_int32_le(UtObject *object, size_t index);

int32_t ut_uint8_list_get_int32_be(UtObject *object, size_t index);

uint64_t ut_uint8_list_get_uint64_le(UtObject *object, size_t index);

uint64_t ut_uint8_list_get_uint64_be(UtObject *object, size_t index);

int64_t ut_uint8_list_get_int64_le(UtObject *object, size_t index);

int64_t ut_uint8_list_get_int64_be(UtObject *object, size_t index);

double ut_uint8_list_get_float64_le(UtObject *object, size_t index);

double ut_uint8_list_get_float64_be(UtObject *object, size_t index);

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

void ut_uint8_list_append_uint64_le(UtObject *object, uint64_t value);

void ut_uint8_list_append_uint64_be(UtObject *object, uint64_t value);

void ut_uint8_list_append_int64_le(UtObject *object, int64_t value);

void ut_uint8_list_append_int64_be(UtObject *object, int64_t value);

void ut_uint8_list_append_float64_le(UtObject *object, double value);

void ut_uint8_list_append_float64_be(UtObject *object, double value);

void ut_uint8_list_prepend(UtObject *object, uint8_t value);

void ut_uint8_list_prepend_block(UtObject *object, const uint8_t *data,
                                 size_t data_length);

void ut_uint8_list_insert(UtObject *object, size_t index, uint8_t value);

void ut_uint8_list_insert_block(UtObject *object, size_t index,
                                const uint8_t *data, size_t data_length);

bool ut_object_implements_uint8_list(UtObject *object);
