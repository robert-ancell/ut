#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_drawable_error_new(uint32_t drawable);

uint32_t ut_x11_drawable_error_get_drawable(UtObject *object);

bool ut_object_is_x11_drawable_error(UtObject *object);
