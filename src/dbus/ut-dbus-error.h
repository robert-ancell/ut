#include <stdarg.h>
#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_error_new(const char *error_name, UtObject *args);

const char *ut_dbus_error_get_error_name(UtObject *object);

UtObject *ut_dbus_error_get_args(UtObject *object);

bool ut_object_is_dbus_error(UtObject *object);
