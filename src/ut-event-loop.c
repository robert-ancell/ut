#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

#include "ut-event-loop.h"
#include "ut-object-private.h"

typedef struct _Timeout Timeout;
struct _Timeout {
  struct timespec when;
  UtTimeoutCallback callback;
  void *user_data;
  Timeout *next;
};

typedef struct {
  Timeout *timeouts;
} UtEventLoop;

static int time_compare(struct timespec *a, struct timespec *b) {
  if (a->tv_sec == b->tv_sec) {
    if (a->tv_nsec == b->tv_nsec) {
      return 0;
    }

    return a->tv_nsec > b->tv_nsec ? 1 : -1;
  }

  return a->tv_sec > b->tv_sec ? 1 : -1;
}

static void time_delta(struct timespec *a, struct timespec *b,
                       struct timespec *delta) {
  delta->tv_sec = b->tv_sec - a->tv_sec;
  if (b->tv_nsec > a->tv_nsec) {
    delta->tv_nsec = b->tv_nsec - a->tv_nsec;
  } else {
    delta->tv_sec--;
    delta->tv_nsec = 1000000000 + b->tv_nsec - a->tv_nsec;
  }
}

static const char *ut_event_loop_get_type_name() { return "EventLoop"; }

static void ut_event_loop_init(UtObject *object) {
  UtEventLoop *self = ut_object_get_data(object);
  self->timeouts = NULL;
}

static void ut_event_loop_cleanup(UtObject *object) {
  UtEventLoop *self = ut_object_get_data(object);
  Timeout *next_timeout;
  for (Timeout *timeout = self->timeouts; timeout != NULL;
       timeout = next_timeout) {
    next_timeout = timeout->next;
    free(timeout);
  }
  self->timeouts = NULL;
}

static UtObjectFunctions object_functions = {.get_type_name =
                                                 ut_event_loop_get_type_name,
                                             .init = ut_event_loop_init,
                                             .cleanup = ut_event_loop_cleanup};

UtObject *ut_event_loop_new() {
  return ut_object_new(sizeof(UtEventLoop), &object_functions);
}

void ut_event_loop_add_timeout(UtObject *object, time_t seconds,
                               UtTimeoutCallback callback, void *user_data) {
  UtEventLoop *self = ut_object_get_data(object);

  Timeout *t = malloc(sizeof(Timeout));
  assert(clock_gettime(CLOCK_MONOTONIC, &t->when) == 0);
  t->when.tv_sec += seconds;
  t->callback = callback;
  t->user_data = user_data;
  t->next = NULL;

  Timeout *prev_timeout = NULL;
  for (Timeout *next_timeout = self->timeouts; next_timeout != NULL;
       next_timeout = next_timeout->next) {
    if (time_compare(&next_timeout->when, &t->when) > 0) {
      break;
    }
    prev_timeout = next_timeout;
  }
  if (prev_timeout != NULL) {
    t->next = prev_timeout->next;
    prev_timeout->next = t;
  } else {
    t->next = self->timeouts;
    self->timeouts = t;
  }
}

void ut_event_loop_run(UtObject *object) {
  UtEventLoop *self = ut_object_get_data(object);
  while (true) {
    const struct timespec *timeout = NULL;
    struct timespec first_timeout;
    while (self->timeouts != NULL && timeout == NULL) {
      Timeout *t = self->timeouts;
      struct timespec now;
      assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);

      if (time_compare(&t->when, &now) <= 0) {
        t->callback(t->user_data);
        self->timeouts = t->next;
        free(t);
      } else {
        time_delta(&now, &t->when, &first_timeout);
        timeout = &first_timeout;
      }
    }

    int n_fds = pselect(0, NULL, NULL, NULL, timeout, NULL);
    if (n_fds < 0) {
      return;
    }
  }
}

bool ut_object_is_event_loop(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
