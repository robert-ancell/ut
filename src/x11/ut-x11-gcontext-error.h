#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_gcontext_error_new(uint32_t gc);

uint32_t ut_x11_gcontext_error_get_gc(UtObject *object);

bool ut_object_is_x11_gcontext_error(UtObject *object);
