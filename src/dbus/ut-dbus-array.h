#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_array_new(const char *value_signature);

const char *ut_dbus_array_get_value_signature(UtObject *object);

bool ut_object_is_dbus_array(UtObject *object);
