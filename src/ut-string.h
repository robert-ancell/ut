#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const char *(*get_text)(UtObject *object);
} UtStringFunctions;

extern int ut_string_id;

const char *ut_string_get_text(UtObject *object);

UtObject *ut_string_get_code_points(UtObject *object);

char *ut_string_to_string(UtObject *object);

bool ut_object_implements_string(UtObject *object);
