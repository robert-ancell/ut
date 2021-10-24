#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-end-of-stream.h"
#include "ut-event-loop.h"
#include "ut-fd-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-output-stream.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  int fd;
  UtObject *read_buffer;
  size_t read_buffer_length;
} UtFdStream;

typedef struct {
  UtFdStream *self;
  size_t block_length;
  bool read_all;
  UtObject *watch_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} ReadData;

typedef struct {
  UtFdStream *self;
  UtObject *data;
  size_t n_written;
  bool write_all;
  UtObject *watch_cancel;
  UtOutputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} WriteData;

static ReadData *read_data_new(UtFdStream *self, size_t block_length,
                               bool read_all, UtInputStreamCallback callback,
                               void *user_data, UtObject *cancel) {
  ReadData *data = malloc(sizeof(ReadData));
  data->self = self;
  data->block_length = block_length;
  data->read_all = read_all;
  data->watch_cancel = ut_cancel_new();
  data->callback = callback;
  data->user_data = user_data;
  data->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  return data;
}

static void read_data_free(ReadData *data) {
  ut_object_unref(data->watch_cancel);
  if (data->cancel) {
    ut_object_unref(data->cancel);
  }
  free(data);
}

static void report_read_data(ReadData *data) {
  UtFdStream *self = data->self;

  // FIXME: Can report more data than requested in a single read - trim buffer
  // in this case.
  ut_list_resize(self->read_buffer, self->read_buffer_length);
  size_t n_used = data->callback(data->user_data, self->read_buffer);
  ut_list_remove(self->read_buffer, 0, n_used);
  self->read_buffer_length -= n_used;
}

static void read_cb(void *user_data) {
  ReadData *data = user_data;
  UtFdStream *self = data->self;

  bool done = false;
  if (data->cancel == NULL || !ut_cancel_is_active(data->cancel)) {
    // Make space to read a new block.
    ut_list_resize(self->read_buffer,
                   self->read_buffer_length + data->block_length);

    // Read a block.
    ssize_t n_read = read(self->fd,
                          ut_uint8_array_get_data(self->read_buffer) +
                              self->read_buffer_length,
                          data->block_length);
    assert(n_read >= 0);
    self->read_buffer_length += n_read;

    if (n_read == 0) {
      if (data->read_all) {
        report_read_data(data);
      } else {
        UtObjectRef end_of_stream = ut_end_of_stream_new(
            self->read_buffer_length > 0 ? self->read_buffer : NULL);
        data->callback(data->user_data, end_of_stream);
      }
      done = true;
    } else if (!data->read_all) {
      report_read_data(data);
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

static void add_read_watch(ReadData *data) {
  UtFdStream *self = data->self;
  ut_event_loop_add_read_watch(self->fd, read_cb, data, data->watch_cancel);
}

static void buffered_read_cb(void *user_data) {
  ReadData *data = user_data;
  if (data->cancel != NULL && ut_cancel_is_active(data->cancel)) {
    return;
  }
  report_read_data(data);
  if (data->cancel != NULL && ut_cancel_is_active(data->cancel)) {
    return;
  }
  add_read_watch(data);
}

static void start_read(UtFdStream *self, size_t block_length, bool read_all,
                       UtInputStreamCallback callback, void *user_data,
                       UtObject *cancel) {
  ReadData *data =
      read_data_new(self, block_length, read_all, callback, user_data, cancel);

  // If have buffered data, process that first.
  if (self->read_buffer_length > 0) {
    ut_event_loop_add_delay(0, buffered_read_cb, data, cancel);
  } else {
    add_read_watch(data);
  }
}

static WriteData *write_data_new(UtFdStream *self, UtObject *data_,
                                 bool write_all,
                                 UtOutputStreamCallback callback,
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
    size_t n_to_write = ut_list_get_length(data->data) - data->n_written;
    const uint8_t *buffer;
    uint8_t *allocated_buffer = NULL;
    if (ut_object_is_uint8_array(data->data)) {
      buffer = ut_uint8_array_get_data(data->data) + data->n_written;
    } else if (ut_object_is_constant_uint8_array(data->data)) {
      buffer = ut_constant_uint8_array_get_data(data->data) + data->n_written;
    } else {
      allocated_buffer = malloc(sizeof(uint8_t) * n_to_write);
      for (size_t i = 0; i < n_to_write; i++) {
        allocated_buffer[i] =
            ut_uint8_list_get_element(data->data, data->n_written + i);
      }
    }
    ssize_t n_written = write(self->fd, buffer, n_to_write);
    if (allocated_buffer != NULL) {
      free(allocated_buffer);
    }
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
  UtFdStream *self = (UtFdStream *)object;
  self->fd = -1;
  self->read_buffer = ut_uint8_array_new();
  self->read_buffer_length = 0;
}

static void ut_fd_stream_cleanup(UtObject *object) {
  UtFdStream *self = (UtFdStream *)object;
  ut_object_unref(self->read_buffer);
}

static void ut_fd_stream_read(UtObject *object, size_t block_size,
                              UtInputStreamCallback callback, void *user_data,
                              UtObject *cancel) {
  UtFdStream *self = (UtFdStream *)object;
  assert(self->fd >= 0);
  assert(callback != NULL);

  start_read(self, block_size, false, callback, user_data, cancel);
}

static void ut_fd_stream_read_all(UtObject *object, size_t block_size,
                                  UtInputStreamCallback callback,
                                  void *user_data, UtObject *cancel) {
  UtFdStream *self = (UtFdStream *)object;
  assert(self->fd >= 0);
  assert(callback != NULL);

  start_read(self, block_size, true, callback, user_data, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_fd_stream_read, .read_all = ut_fd_stream_read_all};

static void ut_fd_stream_write(UtObject *object, UtObject *data,
                               UtOutputStreamCallback callback, void *user_data,
                               UtObject *cancel) {
  UtFdStream *self = (UtFdStream *)object;
  assert(self->fd >= 0);

  WriteData *callback_data =
      write_data_new(self, data, false, callback, user_data, cancel);
  ut_event_loop_add_write_watch(self->fd, write_cb, callback_data,
                                callback_data->watch_cancel);
}

static void ut_fd_stream_write_all(UtObject *object, UtObject *data,
                                   UtOutputStreamCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtFdStream *self = (UtFdStream *)object;
  assert(self->fd >= 0);

  WriteData *callback_data =
      write_data_new(self, data, true, callback, user_data, cancel);
  ut_event_loop_add_write_watch(self->fd, write_cb, callback_data,
                                callback_data->watch_cancel);
}

static UtOutputStreamInterface output_stream_interface = {
    .write = ut_fd_stream_write, .write_all = ut_fd_stream_write_all};

static UtObjectInterface object_interface = {
    .type_name = "UtFdStream",
    .init = ut_fd_stream_init,
    .cleanup = ut_fd_stream_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {&ut_output_stream_id, &output_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_fd_stream_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtFdStream), &object_interface);
  UtFdStream *self = (UtFdStream *)object;
  self->fd = fd;
  return object;
}

bool ut_object_is_fd_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
