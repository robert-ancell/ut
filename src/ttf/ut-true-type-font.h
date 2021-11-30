#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_true_type_font_new();

bool ut_object_is_true_type_font(UtObject *object);
