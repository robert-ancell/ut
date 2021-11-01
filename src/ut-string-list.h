#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const char *(*get_element)(UtObject *object, size_t index);
  char **(*take_data)(UtObject *object);
  void (*insert)(UtObject *object, size_t index, const char *item);
} UtStringListInterface;

extern int ut_string_list_id;

UtObject *ut_string_list_new();

UtObject *ut_string_list_new_with_data(const char *value, ...);

const char *ut_string_list_get_element(UtObject *object, size_t index);

char **ut_string_list_take_data(UtObject *object);

void ut_string_list_append(UtObject *object, const char *item);

void ut_string_list_prepend(UtObject *object, const char *item);

void ut_string_list_insert(UtObject *object, size_t index, const char *item);

bool ut_object_implements_string_list(UtObject *object);
