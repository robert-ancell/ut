#include "ut-object.h"

#pragma once

typedef struct {
  char *(*get_text)(UtObject *object);
  UtObject *(*get_code_points)(UtObject *object);
} UtStringFunctions;

extern int ut_string_id;

char *ut_string_get_text(UtObject *object);

UtObject *ut_string_get_code_points(UtObject *object);
