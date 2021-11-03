#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_key_release_new(uint32_t window, uint8_t keycode, int16_t x,
                                 int16_t y);

uint32_t ut_x11_key_release_get_window(UtObject *object);

uint8_t ut_x11_key_release_get_keycode(UtObject *object);

int16_t ut_x11_key_release_get_x(UtObject *object);

int16_t ut_x11_key_release_get_y(UtObject *object);

bool ut_object_is_x11_key_release(UtObject *object);
