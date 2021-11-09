#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_message_decoder_new(UtObject *input_stream);

bool ut_object_is_dbus_message_decoder(UtObject *object);
