#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_motion_notify_new(uint32_t window, int16_t x, int16_t y);

uint32_t ut_x11_motion_notify_get_window(UtObject *object);

int16_t ut_x11_motion_notify_get_x(UtObject *object);

int16_t ut_x11_motion_notify_get_y(UtObject *object);

bool ut_object_is_x11_motion_notify(UtObject *object);
