#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  const char *(*get_text)(UtObject *object);
} UtUtf8StringFunctions;

extern int ut_utf8_string_id;

const char *ut_utf8_string_get_text(UtObject *object);

UtObject *ut_utf8_string_get_code_points(UtObject *object);

bool ut_object_implements_utf8_string(UtObject *object);
