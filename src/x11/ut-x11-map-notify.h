#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_x11_map_notify_new(uint32_t event, uint32_t window,
                                bool override_redirect);

uint32_t ut_x11_map_notify_get_event(UtObject *object);

uint32_t ut_x11_map_notify_get_window(UtObject *object);

bool ut_x11_map_notify_get_override_redirect(UtObject *object);

bool ut_object_is_x11_map_notify(UtObject *object);
