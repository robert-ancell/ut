#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_http_header_new(const char *name, const char *value);

const char *ut_http_header_get_name(UtObject *object);

const char *ut_http_header_get_value(UtObject *object);

bool ut_object_is_http_header(UtObject *object);
