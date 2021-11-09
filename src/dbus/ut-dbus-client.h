#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtDBusMethodResponseCallback)(void *user_data,
                                             UtObject *out_args);

UtObject *ut_dbus_client_new(const char *address);

UtObject *ut_dbus_client_new_system();

UtObject *ut_dbus_client_new_session();

void ut_dbus_client_call_method(UtObject *object, const char *destination,
                                const char *path, const char *interface,
                                const char *name, UtObject *args,
                                UtDBusMethodResponseCallback callback,
                                void *user_data, UtObject *cancel);

bool ut_object_is_dbus_client(UtObject *object);
