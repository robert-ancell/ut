#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtAuthCompleteCallback)(void *user_data, const char *guid,
                                       UtObject *error);

UtObject *ut_dbus_auth_client_new(UtObject *socket);

void ut_dbus_auth_client_run(UtObject *object, UtAuthCompleteCallback callback,
                             void *user_data, UtObject *cancel);

bool ut_object_is_dbus_auth_client(UtObject *object);
