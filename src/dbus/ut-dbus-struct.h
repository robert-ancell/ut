#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_struct_new(UtObject *value0, ...);

UtObject *ut_dbus_struct_new_take(UtObject *value0, ...);

UtObject *ut_dbus_struct_new_from_list(UtObject *values);

UtObject *ut_dbus_struct_get_value(UtObject *object, size_t index);

UtObject *ut_dbus_struct_get_values(UtObject *object);

bool ut_object_is_dbus_struct(UtObject *object);
