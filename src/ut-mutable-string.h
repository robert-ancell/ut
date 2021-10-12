#include <stddef.h>

#include "ut-object.h"

#pragma once

typedef struct {
  void (*clear)(UtObject *object);
  void (*prepend)(UtObject *object, const char *text);
  void (*append)(UtObject *object, const char *text);
} UtMutableStringFunctions;

extern int ut_mutable_string_id;

void ut_mutable_string_clear(UtObject *object);

void ut_mutable_string_prepend(UtObject *object, const char *text);

void ut_mutable_string_append(UtObject *object, const char *text);
