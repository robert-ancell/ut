#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_fd_stream_new(int fd);

bool ut_object_is_fd_stream(UtObject *object);
