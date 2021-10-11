#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-file.h"
#include "ut-mutable-list.h"
#include "ut-mutable-uint8-list.h"
#include "ut-object-private.h"

typedef struct {
  char *path;
  int fd;
} UtFile;

typedef struct {
  UtFile *self;
  size_t block_length;
  bool read_all;
  bool accumulate;
  UtObject *watch_cancel;
  UtObject *buffer;
  size_t length;
  UtFileReadCallback callback;
  void *user_data;
  UtObject *cancel;
} UtReadData;

UtReadData *ut_read_data_new(UtFile *self, size_t block_length, bool read_all,
                             bool accumulate, UtFileReadCallback callback,
                             void *user_data, UtObject *cancel) {
  UtReadData *data = malloc(sizeof(UtReadData));
  data->self = self;
  data->block_length = block_length;
  data->read_all = read_all;
  data->accumulate = accumulate;
  data->watch_cancel = ut_cancel_new();
  data->buffer = ut_mutable_uint8_list_new();
  data->length = 0;
  data->callback = callback;
  data->user_data = user_data;
  data->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  return data;
}

static void ut_read_data_free(UtReadData *data) {
  ut_object_unref(data->watch_cancel);
  ut_object_unref(data->buffer);
  if (data->cancel) {
    ut_object_unref(data->cancel);
  }
  free(data);
}

static void read_cb(void *user_data) {
  UtReadData *data = user_data;
  UtFile *self = data->self;

  bool done = false;
  if (data->cancel == NULL || !ut_cancel_is_active(data->cancel)) {
    // Make space to read a new block.
    ut_mutable_list_resize(data->buffer, data->length + data->block_length);

    // Read a block.
    ssize_t n_read = read(
        self->fd, ut_mutable_uint8_list_get_data(data->buffer) + data->length,
        data->block_length);
    assert(n_read >= 0);
    data->length += n_read;

    // Done if EOF or only doing single read.
    done = n_read == 0 || !data->read_all;

    // Report either the final data or the read block.
    if (done || !data->accumulate) {
      ut_mutable_list_resize(data->buffer, data->length);
      data->callback(data->user_data, data->buffer);
      if (!data->accumulate) {
        ut_mutable_list_clear(data->buffer);
        data->length = 0;
      }
    }
  } else {
    done = true;
  }

  // Stop listening for read events when done.
  if (done) {
    ut_cancel_activate(data->watch_cancel);
    ut_read_data_free(data);
  }
}

static void close_file(UtFile *self) {
  if (self->fd >= 0) {
    close(self->fd);
    self->fd = -1;
  }
}

static void ut_file_init(UtObject *object) {
  UtFile *self = ut_object_get_data(object);
  self->path = NULL;
  self->fd = -1;
}

static void ut_file_cleanup(UtObject *object) {
  UtFile *self = ut_object_get_data(object);
  free(self->path);
  self->path = NULL;
  close_file(self);
}

static UtObjectFunctions object_functions = {.init = ut_file_init,
                                             .cleanup = ut_file_cleanup};

UtObject *ut_file_new(const char *path) {
  UtObject *object = ut_object_new(sizeof(UtFile), &object_functions);
  UtFile *self = ut_object_get_data(object);
  self->path = strdup(path);
  return object;
}

void ut_file_open_read(UtObject *object) {
  assert(ut_object_is_file(object));
  UtFile *self = ut_object_get_data(object);
  assert(self->fd == -1);
  self->fd = open(self->path, O_RDONLY);
}

void ut_file_read(UtObject *object, size_t count, UtFileReadCallback callback,
                  void *user_data, UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  UtObject *loop = ut_event_loop_get();
  UtReadData *data =
      ut_read_data_new(self, count, false, false, callback, user_data, cancel);
  ut_event_loop_add_read_watch(loop, self->fd, read_cb, data,
                               data->watch_cancel);
}

void ut_file_read_stream(UtObject *object, size_t block_size,
                         UtFileReadCallback callback, void *user_data,
                         UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  UtObject *loop = ut_event_loop_get();
  UtReadData *data = ut_read_data_new(self, block_size, true, false, callback,
                                      user_data, cancel);
  ut_event_loop_add_read_watch(loop, self->fd, read_cb, data,
                               data->watch_cancel);
}

void ut_file_read_all(UtObject *object, size_t block_size,
                      UtFileReadCallback callback, void *user_data,
                      UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  UtObject *loop = ut_event_loop_get();
  UtReadData *data = ut_read_data_new(self, block_size, true, true, callback,
                                      user_data, cancel);
  ut_event_loop_add_read_watch(loop, self->fd, read_cb, data,
                               data->watch_cancel);
}

void ut_file_close(UtObject *object) {
  assert(ut_object_is_file(object));
  UtFile *self = ut_object_get_data(object);
  close_file(self);
}

bool ut_object_is_file(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
