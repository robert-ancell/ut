#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef struct {
  char *(*get_description)(UtObject *object);
} UtErrorInterface;

extern int ut_error_id;

UtObject *ut_error_new(const char *format, ...);

char *ut_error_get_description(UtObject *object);

bool ut_object_implements_error(UtObject *object);
