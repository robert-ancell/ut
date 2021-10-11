#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-object-private.h"

typedef struct _Timeout Timeout;
struct _Timeout {
  struct timespec when;
  struct timespec frequency;
  UtDelayCallback callback;
  void *user_data;
  UtObject *cancel;
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

static void free_timeout(Timeout *timeout) {
  if (timeout->cancel != NULL) {
    ut_object_unref(timeout->cancel);
  }
  free(timeout);
}

static void insert_timeout(UtEventLoop *self, Timeout *timeout) {
  Timeout *prev_timeout = NULL;
  for (Timeout *next_timeout = self->timeouts; next_timeout != NULL;
       next_timeout = next_timeout->next) {
    if (time_compare(&next_timeout->when, &timeout->when) > 0) {
      break;
    }
    prev_timeout = next_timeout;
  }
  if (prev_timeout != NULL) {
    timeout->next = prev_timeout->next;
    prev_timeout->next = timeout;
  } else {
    timeout->next = self->timeouts;
    self->timeouts = timeout;
  }
}

static void add_timeout(UtEventLoop *self, time_t seconds, bool repeat,
                        UtDelayCallback callback, void *user_data,
                        UtObject *cancel) {
  Timeout *t = malloc(sizeof(Timeout));
  assert(clock_gettime(CLOCK_MONOTONIC, &t->when) == 0);
  t->when.tv_sec += seconds;
  t->frequency.tv_sec = repeat ? seconds : 0;
  t->frequency.tv_nsec = 0;
  t->callback = callback;
  t->user_data = user_data;
  t->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  t->next = NULL;

  insert_timeout(self, t);
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
    free_timeout(timeout);
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

void ut_event_loop_add_delay(UtObject *object, time_t seconds,
                             UtDelayCallback callback, void *user_data,
                             UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  add_timeout(self, seconds, false, callback, user_data, cancel);
}

void ut_event_loop_add_timer(UtObject *object, time_t seconds,
                             UtDelayCallback callback, void *user_data,
                             UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  add_timeout(self, seconds, true, callback, user_data, cancel);
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

      bool is_cancelled = t->cancel != NULL && ut_cancel_is_active(t->cancel);
      if (is_cancelled || time_compare(&t->when, &now) <= 0) {
        if (!is_cancelled) {
          t->callback(t->user_data);
        }

        self->timeouts = t->next;
        t->next = NULL;

        bool repeats = t->frequency.tv_sec != 0 || t->frequency.tv_nsec != 0;
        if (is_cancelled || !repeats) {
          free_timeout(t);
        } else {
          t->when.tv_sec += t->frequency.tv_sec;
          t->when.tv_nsec += t->frequency.tv_nsec;
          if (t->when.tv_nsec > 1000000000) {
            t->when.tv_sec++;
            t->when.tv_nsec -= 1000000000;
          }
          insert_timeout(self, t);
        }
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
