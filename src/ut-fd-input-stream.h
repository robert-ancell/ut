#include <stdbool.h>
#include <stddef.h>

#include "ut-object.h"

#pragma once

UtObject *ut_fd_input_stream_new(UtObject *fd);

void ut_fd_input_stream_set_receive_fds(UtObject *object, bool receive_fds);

bool ut_object_is_fd_input_stream(UtObject *object);
