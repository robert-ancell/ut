#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_configure_notify_new(uint32_t window, int16_t x, int16_t y,
                                      uint16_t width, uint16_t height);

uint32_t ut_x11_configure_notify_get_window(UtObject *object);

int16_t ut_x11_configure_notify_get_x(UtObject *object);

int16_t ut_x11_configure_notify_get_y(UtObject *object);

uint16_t ut_x11_configure_notify_get_width(UtObject *object);

uint16_t ut_x11_configure_notify_get_height(UtObject *object);

bool ut_object_is_x11_configure_notify(UtObject *object);
