#include <stdbool.h>
#include <sys/time.h>

#include "ut-object.h"

#pragma once

typedef void (*UtDelayCallback)(void *user_data);

UtObject *ut_event_loop_new();

void ut_event_loop_add_delay(UtObject *object, time_t seconds,
                             UtDelayCallback callback, void *user_data,
                             UtObject *cancel);

void ut_event_loop_add_timer(UtObject *object, time_t seconds,
                             UtDelayCallback callback, void *user_data,
                             UtObject *cancel);

void ut_event_loop_run(UtObject *object);

bool ut_object_is_event_loop(UtObject *object);
