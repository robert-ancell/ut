#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "ut-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

int ut_uint8_list_id = 0;

UtObject *ut_uint8_list_new() { return ut_uint8_array_new(); }

UtObject *ut_uint8_list_new_with_data(size_t length, ...) {
  va_list ap;
  va_start(ap, length);
  return ut_uint8_array_new_with_va_data(length, ap);
  va_end(ap);
}

uint8_t ut_uint8_list_get_element(UtObject *object, size_t index) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->get_element(object, index);
}

int8_t ut_uint8_list_get_int8(UtObject *object, size_t index) {
  return (int8_t)ut_uint8_list_get_element(object, index);
}

uint16_t ut_uint8_list_get_uint16_le(UtObject *object, size_t index) {
  return (uint16_t)ut_uint8_list_get_element(object, index) |
         (uint16_t)ut_uint8_list_get_element(object, index + 1) << 8;
}

uint16_t ut_uint8_list_get_uint16_be(UtObject *object, size_t index) {
  return (uint16_t)ut_uint8_list_get_element(object, index) << 8 |
         (uint16_t)ut_uint8_list_get_element(object, index + 1);
}

int16_t ut_uint8_list_get_int16_le(UtObject *object, size_t index) {
  return (int16_t)ut_uint8_list_get_uint16_le(object, index);
}

int16_t ut_uint8_list_get_int16_be(UtObject *object, size_t index) {
  return (int16_t)ut_uint8_list_get_uint16_be(object, index);
}

uint32_t ut_uint8_list_get_uint32_le(UtObject *object, size_t index) {
  return (uint32_t)ut_uint8_list_get_element(object, index) |
         (uint32_t)ut_uint8_list_get_element(object, index + 1) << 8 |
         (uint32_t)ut_uint8_list_get_element(object, index + 2) << 16 |
         (uint32_t)ut_uint8_list_get_element(object, index + 3) << 24;
}

uint32_t ut_uint8_list_get_uint32_be(UtObject *object, size_t index) {
  return (uint32_t)ut_uint8_list_get_element(object, index) << 24 |
         (uint32_t)ut_uint8_list_get_element(object, index + 1) << 16 |
         (uint32_t)ut_uint8_list_get_element(object, index + 2) << 16 |
         (uint32_t)ut_uint8_list_get_element(object, index + 3);
}

int32_t ut_uint8_list_get_int32_le(UtObject *object, size_t index) {
  return (int32_t)ut_uint8_list_get_uint32_le(object, index);
}

int32_t ut_uint8_list_get_int32_be(UtObject *object, size_t index) {
  return (int32_t)ut_uint8_list_get_uint32_be(object, index);
}

uint64_t ut_uint8_list_get_uint64_le(UtObject *object, size_t index) {
  return (uint64_t)ut_uint8_list_get_element(object, index) |
         (uint64_t)ut_uint8_list_get_element(object, index + 1) << 8 |
         (uint64_t)ut_uint8_list_get_element(object, index + 2) << 16 |
         (uint64_t)ut_uint8_list_get_element(object, index + 3) << 24 |
         (uint64_t)ut_uint8_list_get_element(object, index + 4) << 32 |
         (uint64_t)ut_uint8_list_get_element(object, index + 5) << 40 |
         (uint64_t)ut_uint8_list_get_element(object, index + 6) << 48 |
         (uint64_t)ut_uint8_list_get_element(object, index + 7) << 56;
}

uint64_t ut_uint8_list_get_uint64_be(UtObject *object, size_t index) {
  return (uint64_t)ut_uint8_list_get_element(object, index) << 56 |
         (uint64_t)ut_uint8_list_get_element(object, index + 1) << 48 |
         (uint64_t)ut_uint8_list_get_element(object, index + 2) << 40 |
         (uint64_t)ut_uint8_list_get_element(object, index + 3) << 32 |
         (uint64_t)ut_uint8_list_get_element(object, index + 4) << 24 |
         (uint64_t)ut_uint8_list_get_element(object, index + 5) << 16 |
         (uint64_t)ut_uint8_list_get_element(object, index + 6) << 8 |
         (uint64_t)ut_uint8_list_get_element(object, index + 7);
}

int64_t ut_uint8_list_get_int64_le(UtObject *object, size_t index) {
  return (int64_t)ut_uint8_list_get_uint64_le(object, index);
}

int64_t ut_uint8_list_get_int64_be(UtObject *object, size_t index) {
  return (int64_t)ut_uint8_list_get_uint64_be(object, index);
}

double ut_uint8_list_get_float64_le(UtObject *object, size_t index) {
  uint64_t value = ut_uint8_list_get_uint64_le(object, index);
  double *value_pointer = (double *)&value;
  return *value_pointer;
}

double ut_uint8_list_get_float64_be(UtObject *object, size_t index) {
  uint64_t value = ut_uint8_list_get_uint64_be(object, index);
  double *value_pointer = (double *)&value;
  return *value_pointer;
}

uint8_t *ut_uint8_list_take_data(UtObject *object) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  return uint8_list_interface->take_data(object);
}

void ut_uint8_list_append(UtObject *object, uint8_t value) {
  ut_uint8_list_append_block(object, &value, 1);
}

void ut_uint8_list_append_block(UtObject *object, const uint8_t *data,
                                size_t data_length) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  uint8_list_interface->append(object, data, data_length);
}

void ut_uint8_list_append_int8(UtObject *object, int8_t value) {
  ut_uint8_list_append(object, (uint8_t)value);
}

void ut_uint8_list_append_uint16_le(UtObject *object, uint16_t value) {
  uint8_t data[2] = {value & 0xff, value >> 8};
  ut_uint8_list_append_block(object, data, 2);
}

void ut_uint8_list_append_uint16_be(UtObject *object, uint16_t value) {
  uint8_t data[2] = {value >> 8, value & 0xff};
  ut_uint8_list_append_block(object, data, 2);
}

void ut_uint8_list_append_int16_le(UtObject *object, int16_t value) {
  ut_uint8_list_append_int16_le(object, (uint16_t)value);
}

void ut_uint8_list_append_int16_be(UtObject *object, int16_t value) {
  ut_uint8_list_append_int16_be(object, (uint16_t)value);
}

void ut_uint8_list_append_uint32_le(UtObject *object, uint32_t value) {
  uint8_t data[4] = {value & 0xff, (value >> 8) & 0xff, (value >> 16) & 0xff,
                     value >> 24};
  ut_uint8_list_append_block(object, data, 4);
}

void ut_uint8_list_append_uint32_be(UtObject *object, uint32_t value) {
  uint8_t data[4] = {value >> 24, (value >> 16) & 0xff, (value >> 8) & 0xff,
                     value & 0xff};
  ut_uint8_list_append_block(object, data, 4);
}

void ut_uint8_list_append_int32_le(UtObject *object, int32_t value) {
  ut_uint8_list_append_int32_le(object, (uint32_t)value);
}

void ut_uint8_list_append_int32_be(UtObject *object, int32_t value) {
  ut_uint8_list_append_int32_be(object, (uint32_t)value);
}

void ut_uint8_list_append_uint64_le(UtObject *object, uint64_t value) {
  uint8_t data[8] = {value & 0xff,         (value >> 8) & 0xff,
                     (value >> 16) & 0xff, (value >> 24) & 0xff,
                     (value >> 32) & 0xff, (value >> 48) & 0xff,
                     value >> 56};
  ut_uint8_list_append_block(object, data, 8);
}

void ut_uint8_list_append_uint64_be(UtObject *object, uint64_t value) {
  uint8_t data[8] = {value >> 56,          (value >> 48) & 0xff,
                     (value >> 40) & 0xff, (value >> 32) & 0xff,
                     (value >> 24) & 0xff, (value >> 16) & 0xff,
                     (value >> 8) & 0xff,  value & 0xff};
  ut_uint8_list_append_block(object, data, 8);
}

void ut_uint8_list_append_int64_le(UtObject *object, int64_t value) {
  ut_uint8_list_append_int64_le(object, (uint64_t)value);
}

void ut_uint8_list_append_int64_be(UtObject *object, int64_t value) {
  ut_uint8_list_append_int64_be(object, (uint64_t)value);
}

void ut_uint8_list_append_float64_le(UtObject *object, double value) {
  uint64_t *value_pointer = (uint64_t *)&value;
  ut_uint8_list_append_uint64_le(object, *value_pointer);
}

void ut_uint8_list_append_float64_be(UtObject *object, double value) {
  uint64_t *value_pointer = (uint64_t *)&value;
  ut_uint8_list_append_uint64_be(object, *value_pointer);
}

void ut_uint8_list_prepend(UtObject *object, uint8_t value) {
  ut_uint8_list_prepend_block(object, &value, 1);
}

void ut_uint8_list_prepend_block(UtObject *object, const uint8_t *data,
                                 size_t data_length) {
  ut_uint8_list_insert_block(object, 0, data, data_length);
}

void ut_uint8_list_insert(UtObject *object, size_t index, uint8_t value) {
  ut_uint8_list_insert_block(object, index, &value, 1);
}

void ut_uint8_list_insert_block(UtObject *object, size_t index,
                                const uint8_t *data, size_t data_length) {
  UtUint8ListInterface *uint8_list_interface =
      ut_object_get_interface(object, &ut_uint8_list_id);
  assert(uint8_list_interface != NULL);
  assert(ut_list_is_mutable(object));
  uint8_list_interface->insert(object, index, data, data_length);
}

bool ut_object_implements_uint8_list(UtObject *object) {
  return ut_object_get_interface(object, &ut_uint8_list_id) != NULL;
}
