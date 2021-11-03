#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_id_choice_error_new(uint32_t resource_id);

uint32_t ut_x11_id_choice_error_get_resource_id(UtObject *object);

bool ut_object_is_x11_id_choice_error(UtObject *object);
