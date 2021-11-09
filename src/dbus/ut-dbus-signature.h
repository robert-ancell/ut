#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_signature_new(const char *value);

const char *ut_dbus_signature_get_value(UtObject *object);

UtObject *ut_dbus_signature_split(UtObject *object);

bool ut_object_is_dbus_signature(UtObject *object);
