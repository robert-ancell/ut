#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-fd-stream.h"
#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-uint8-list.h"
#include "ut-object-private.h"
#include "ut-uint8-list.h"

typedef struct {
  int fd;
} UtFdStream;

typedef struct {
  UtFdStream *self;
  size_t block_length;
  bool read_all;
  bool accumulate;
  UtObject *watch_cancel;
  UtObject *buffer;
  size_t length;
  UtFdStreamReadCallback callback;
  void *user_data;
  UtObject *cancel;
} ReadData;

typedef struct {
  UtFdStream *self;
  UtObject *data;
  size_t n_written;
  bool write_all;
  UtObject *watch_cancel;
  UtFdStreamWriteCallback callback;
  void *user_data;
  UtObject *cancel;
} WriteData;

static ReadData *read_data_new(UtFdStream *self, size_t block_length,
                               bool read_all, bool accumulate,
                               UtFdStreamReadCallback callback, void *user_data,
                               UtObject *cancel) {
  ReadData *data = malloc(sizeof(ReadData));
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

static void read_data_free(ReadData *data) {
  ut_object_unref(data->watch_cancel);
  ut_object_unref(data->buffer);
  if (data->cancel) {
    ut_object_unref(data->cancel);
  }
  free(data);
}

static void read_cb(void *user_data) {
  ReadData *data = user_data;
  UtFdStream *self = data->self;

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
      if (data->callback != NULL) {
        data->callback(data->user_data, data->buffer);
      }
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
    read_data_free(data);
  }
}

static WriteData *write_data_new(UtFdStream *self, UtObject *data_,
                                 bool write_all,
                                 UtFdStreamWriteCallback callback,
                                 void *user_data, UtObject *cancel) {
  WriteData *data = malloc(sizeof(WriteData));
  data->self = self;
  data->data = ut_object_ref(data_);
  data->n_written = 0;
  data->write_all = write_all;
  data->watch_cancel = ut_cancel_new();
  data->callback = callback;
  data->user_data = user_data;
  data->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  return data;
}

static void write_data_free(WriteData *data) {
  ut_object_unref(data->data);
  ut_object_unref(data->watch_cancel);
  if (data->cancel) {
    ut_object_unref(data->cancel);
  }
  free(data);
}

static void write_cb(void *user_data) {
  WriteData *data = user_data;
  UtFdStream *self = data->self;

  bool done = false;
  if (data->cancel == NULL || !ut_cancel_is_active(data->cancel)) {
    // Write remaining data.
    ssize_t n_written =
        write(self->fd, ut_uint8_list_get_data(data->data) + data->n_written,
              ut_list_get_length(data->data) - data->n_written);
    assert(n_written >= 0);
    data->n_written += n_written;

    // Done if all data written or only doing single write.
    done =
        data->n_written == ut_list_get_length(data->data) || !data->write_all;

    // Report how much data was written.
    if (done && data->callback != NULL) {
      data->callback(data->user_data, data->n_written);
    }
  } else {
    done = true;
  }

  // Stop listening for read events when done.
  if (done) {
    ut_cancel_activate(data->watch_cancel);
    write_data_free(data);
  }
}

static void ut_fd_stream_init(UtObject *object) {
  UtFdStream *self = ut_object_get_data(object);
  self->fd = -1;
}

static UtObjectFunctions object_functions = {
    .init = ut_fd_stream_init,
};

UtObject *ut_fd_stream_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtFdStream), &object_functions);
  UtFdStream *self = ut_object_get_data(object);
  self->fd = fd;
  return object;
}

void ut_fd_stream_read(UtObject *object, size_t count,
                       UtFdStreamReadCallback callback, void *user_data,
                       UtObject *cancel) {
  UtFdStream *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  ReadData *data =
      read_data_new(self, count, false, false, callback, user_data, cancel);
  ut_event_loop_add_read_watch(self->fd, read_cb, data, data->watch_cancel);
}

void ut_fd_stream_read_stream(UtObject *object, size_t block_size,
                              UtFdStreamReadCallback callback, void *user_data,
                              UtObject *cancel) {
  UtFdStream *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  ReadData *data =
      read_data_new(self, block_size, true, false, callback, user_data, cancel);
  ut_event_loop_add_read_watch(self->fd, read_cb, data, data->watch_cancel);
}

void ut_fd_stream_read_all(UtObject *object, size_t block_size,
                           UtFdStreamReadCallback callback, void *user_data,
                           UtObject *cancel) {
  UtFdStream *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  ReadData *data =
      read_data_new(self, block_size, true, true, callback, user_data, cancel);
  ut_event_loop_add_read_watch(self->fd, read_cb, data, data->watch_cancel);
}

void ut_fd_stream_write(UtObject *object, UtObject *data,
                        UtFdStreamWriteCallback callback, void *user_data,
                        UtObject *cancel) {
  UtFdStream *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  WriteData *callback_data =
      write_data_new(self, data, false, callback, user_data, cancel);
  ut_event_loop_add_write_watch(self->fd, write_cb, callback_data,
                                callback_data->watch_cancel);
}

void ut_fd_stream_write_all(UtObject *object, UtObject *data,
                            UtFdStreamWriteCallback callback, void *user_data,
                            UtObject *cancel) {
  UtFdStream *self = ut_object_get_data(object);
  assert(self->fd >= 0);

  WriteData *callback_data =
      write_data_new(self, data, true, callback, user_data, cancel);
  ut_event_loop_add_write_watch(self->fd, write_cb, callback_data,
                                callback_data->watch_cancel);
}

bool ut_object_is_fd_stream(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}