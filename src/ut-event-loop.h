#include <stdbool.h>
#include <sys/time.h>

#include "ut-object.h"

#pragma once

typedef void (*UtTimeoutCallback)(void *user_data);

UtObject *ut_event_loop_new();

void ut_event_loop_add_timeout(UtObject *object, time_t seconds,
                               UtTimeoutCallback callback, void *user_data);

void ut_event_loop_run(UtObject *object);

bool ut_object_is_event_loop(UtObject *object);
