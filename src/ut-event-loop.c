#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-object-private.h"

static UtObject *loop = NULL;

typedef struct _Timeout Timeout;
struct _Timeout {
  struct timespec when;
  struct timespec frequency;
  UtEventLoopCallback callback;
  void *user_data;
  UtObject *cancel;
  Timeout *next;
};

typedef struct _FdWatch FdWatch;
struct _FdWatch {
  int fd;
  UtEventLoopCallback callback;
  void *user_data;
  UtObject *cancel;
  FdWatch *next;
};

typedef struct {
  Timeout *timeouts;
  FdWatch *read_watches;
  FdWatch *write_watches;
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
                        UtEventLoopCallback callback, void *user_data,
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

static FdWatch *fd_watch_new(int fd, UtEventLoopCallback callback,
                             void *user_data, UtObject *cancel) {
  FdWatch *watch = malloc(sizeof(FdWatch));
  watch->fd = fd;
  watch->callback = callback;
  watch->user_data = user_data;
  watch->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  watch->next = NULL;
  return watch;
}

static void free_fd_watch(FdWatch *watch) {
  if (watch->cancel != NULL) {
    ut_object_unref(watch->cancel);
  }
  free(watch);
}

static FdWatch *remove_cancelled_watches(FdWatch *watches) {
  FdWatch *prev_watch = NULL, *next_watch;
  for (FdWatch *watch = watches; watch != NULL; watch = next_watch) {
    next_watch = watch->next;
    if (watch->cancel != NULL && ut_cancel_is_active(watch->cancel)) {
      if (prev_watch != NULL) {
        prev_watch->next = watch->next;
      } else {
        watches->next = watch->next;
      }
      watch->next = NULL;
      free_fd_watch(watch);
    } else {
      prev_watch = watch;
    }
  }

  return watches;
}

static const char *ut_event_loop_get_type_name() { return "EventLoop"; }

static void ut_event_loop_init(UtObject *object) {
  UtEventLoop *self = ut_object_get_data(object);
  self->timeouts = NULL;
  self->read_watches = NULL;
  self->write_watches = NULL;
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
  FdWatch *next_watch;
  for (FdWatch *watch = self->read_watches; watch != NULL; watch = next_watch) {
    next_watch = watch->next;
    free_fd_watch(watch);
  }
  self->read_watches = NULL;
  for (FdWatch *watch = self->write_watches; watch != NULL;
       watch = next_watch) {
    next_watch = watch->next;
    free_fd_watch(watch);
  }
  self->write_watches = NULL;
}

static UtObjectFunctions object_functions = {.get_type_name =
                                                 ut_event_loop_get_type_name,
                                             .init = ut_event_loop_init,
                                             .cleanup = ut_event_loop_cleanup};

UtObject *ut_event_loop_get() {
  // FIXME: Check if this loop is for another thread, and make a new loop if so.
  if (loop == NULL) {
    loop = ut_object_new(sizeof(UtEventLoop), &object_functions);
  }
  return loop;
}

void ut_event_loop_add_delay(UtObject *object, time_t seconds,
                             UtEventLoopCallback callback, void *user_data,
                             UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  add_timeout(self, seconds, false, callback, user_data, cancel);
}

void ut_event_loop_add_timer(UtObject *object, time_t seconds,
                             UtEventLoopCallback callback, void *user_data,
                             UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  add_timeout(self, seconds, true, callback, user_data, cancel);
}

void ut_event_loop_add_read_watch(UtObject *object, int fd,
                                  UtEventLoopCallback callback, void *user_data,
                                  UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  FdWatch *watch = fd_watch_new(fd, callback, user_data, cancel);
  watch->next = self->read_watches;
  self->read_watches = watch;
}

void ut_event_loop_add_write_watch(UtObject *object, int fd,
                                   UtEventLoopCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtEventLoop *self = ut_object_get_data(object);
  FdWatch *watch = fd_watch_new(fd, callback, user_data, cancel);
  watch->next = self->write_watches;
  self->write_watches = watch;
}

void ut_event_loop_run(UtObject *object) {
  UtEventLoop *self = ut_object_get_data(object);
  while (true) {
    // Do callbacks for any timers that have expired and work out time to next
    // timer.
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

    // Register file descriptors we are watching for.
    int max_fd = -1;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    self->read_watches = remove_cancelled_watches(self->read_watches);
    for (FdWatch *watch = self->read_watches; watch != NULL;
         watch = watch->next) {
      FD_SET(watch->fd, &read_fds);
      if (watch->fd > max_fd) {
        max_fd = watch->fd;
      }
    }
    fd_set write_fds;
    FD_ZERO(&write_fds);
    self->write_watches = remove_cancelled_watches(self->write_watches);
    for (FdWatch *watch = self->write_watches; watch != NULL;
         watch = watch->next) {
      FD_SET(watch->fd, &write_fds);
      if (watch->fd > max_fd) {
        max_fd = watch->fd;
      }
    }

    // Wait for file descriptors or timeout.
    assert(pselect(max_fd + 1, &read_fds, &write_fds, NULL, timeout, NULL) >=
           0);

    // Do callbacks for each fd that has changed.
    // Note they are checked for cancellation as they might be cancelled in
    // these callbacks.
    for (FdWatch *watch = self->read_watches; watch != NULL;
         watch = watch->next) {
      bool is_cancelled =
          watch->cancel != NULL && ut_cancel_is_active(watch->cancel);
      if (!is_cancelled && FD_ISSET(watch->fd, &read_fds)) {
        watch->callback(watch->user_data);
      }
    }
    for (FdWatch *watch = self->write_watches; watch != NULL;
         watch = watch->next) {
      bool is_cancelled =
          watch->cancel != NULL && ut_cancel_is_active(watch->cancel);
      if (!is_cancelled && FD_ISSET(watch->fd, &write_fds)) {
        watch->callback(watch->user_data);
      }
    }

    // Purge any watches that have been removed.
    self->read_watches = remove_cancelled_watches(self->read_watches);
    self->write_watches = remove_cancelled_watches(self->write_watches);
  }
}

bool ut_object_is_event_loop(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
