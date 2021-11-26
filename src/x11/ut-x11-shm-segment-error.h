#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_shm_segment_error_new(uint32_t segment);

uint32_t ut_x11_shm_segment_error_get_segment(UtObject *object);

bool ut_object_is_x11_shm_segment_error(UtObject *object);
