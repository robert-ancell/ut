#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtAuthCompleteCallback)(void *user_data, const char *guid,
                                       bool unix_fd_supported, UtObject *error);

UtObject *ut_dbus_auth_client_new(UtObject *input_stream,
                                  UtObject *output_strea);

void ut_dbus_auth_client_set_negotiate_unix_fd(UtObject *object,
                                               bool negotiate_unix_fd);

void ut_dbus_auth_client_run(UtObject *object, UtAuthCompleteCallback callback,
                             void *user_data, UtObject *cancel);

bool ut_object_is_dbus_auth_client(UtObject *object);
