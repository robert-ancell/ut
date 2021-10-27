#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  double (*get_element)(UtObject *object, size_t index);
  double *(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, double item);
} UtFloat64ListInterface;

extern int ut_float64_list_id;

UtObject *ut_float64_list_new();

UtObject *ut_float64_list_new_with_data(size_t length, ...);

double ut_float64_list_get_element(UtObject *object, size_t index);

void ut_float64_list_append(UtObject *object, double item);

void ut_float64_list_prepend(UtObject *object, double item);

void ut_float64_list_insert(UtObject *object, size_t index, double item);

bool ut_object_implements_float64_list(UtObject *object);
