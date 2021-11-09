#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_object_path_new(const char *value);

const char *ut_dbus_object_path_get_value(UtObject *object);

bool ut_object_is_dbus_object_path(UtObject *object);
