#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_pixmap_error_new(uint32_t pixmap);

uint32_t ut_x11_pixmap_error_get_pixmap(UtObject *object);

bool ut_object_is_x11_pixmap_error(UtObject *object);
