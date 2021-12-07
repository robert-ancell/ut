#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_dbus_unix_fd_new(int fd);

int ut_dbus_unix_fd_get_fd(UtObject *object);

int ut_dbus_unix_fd_take_fd(UtObject *object);

bool ut_object_is_dbus_unix_fd(UtObject *object);
