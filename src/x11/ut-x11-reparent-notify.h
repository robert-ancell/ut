#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_reparent_notify_new(uint32_t event, uint32_t window,
                                     uint32_t parent, int16_t x, int16_t y,
                                     bool override_redirect);

uint32_t ut_x11_reparent_notify_get_event(UtObject *object);

uint32_t ut_x11_reparent_notify_get_window(UtObject *object);

uint32_t ut_x11_reparent_notify_get_parent(UtObject *object);

int16_t ut_x11_reparent_notify_get_x(UtObject *object);

int16_t ut_x11_reparent_notify_get_y(UtObject *object);

bool ut_x11_reparent_notify_get_override_redirect(UtObject *object);

bool ut_object_is_x11_reparent_notify(UtObject *object);
