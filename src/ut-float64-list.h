#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  double (*get_element)(UtObject *object, size_t index);
  double *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const double *data,
                 size_t data_length);
} UtFloat64ListInterface;

extern int ut_float64_list_id;

UtObject *ut_float64_list_new();

UtObject *ut_float64_list_new_with_elements(size_t length, ...);

double ut_float64_list_get_element(UtObject *object, size_t index);

double *ut_float64_list_take_data(UtObject *object);

void ut_float64_list_append(UtObject *object, double item);

void ut_float64_list_append_block(UtObject *object, const double *data,
                                  size_t data_length);

void ut_float64_list_prepend(UtObject *object, double item);

void ut_float64_list_prepend_block(UtObject *object, const double *data,
                                   size_t data_length);

void ut_float64_list_insert(UtObject *object, size_t index, double item);

void ut_float64_list_insert_block(UtObject *object, size_t index,
                                  const double *data, size_t data_length);

bool ut_object_implements_float64_list(UtObject *object);
