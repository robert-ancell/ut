#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_colormap_error_new(uint32_t colormap);

uint32_t ut_x11_colormap_error_get_colormap(UtObject *object);

bool ut_object_is_x11_colormap_error(UtObject *object);
