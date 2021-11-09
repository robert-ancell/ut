#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_message_encoder_new();

UtObject *ut_dbus_message_encoder_encode(UtObject *object, UtObject *message);

bool ut_object_is_dbus_message_encoder(UtObject *object);
