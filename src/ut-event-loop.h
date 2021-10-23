#include <stdbool.h>
#include <sys/time.h>

#include "ut-object.h"

#pragma once

typedef void (*UtEventLoopCallback)(void *user_data);
typedef void *(*UtThreadCallback)(void *data);
typedef void (*UtThreadResultCallback)(void *user_data, void *result);

void ut_event_loop_add_delay(time_t seconds, UtEventLoopCallback callback,
                             void *user_data, UtObject *cancel);

void ut_event_loop_add_timer(time_t seconds, UtEventLoopCallback callback,
                             void *user_data, UtObject *cancel);

void ut_event_loop_add_read_watch(int fd, UtEventLoopCallback callback,
                                  void *user_data, UtObject *cancel);

void ut_event_loop_add_write_watch(int fd, UtEventLoopCallback callback,
                                   void *user_data, UtObject *cancel);

void ut_event_loop_add_worker_thread(UtThreadCallback thread_callback,
                                     void *thread_data,
                                     UtEventLoopCallback thread_data_cleanup,
                                     UtThreadResultCallback result_callback,
                                     void *user_data, UtObject *cancel);

void ut_event_loop_return(UtObject *object);

UtObject *ut_event_loop_run();
