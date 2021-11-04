#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-end-of-stream.h"
#include "ut-event-loop.h"
#include "ut-fd-input-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  int fd;
  UtObject *read_buffer;
  size_t read_buffer_length;
  size_t block_size;
  bool read_all;
  UtObject *watch_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} UtFdInputStream;

static void report_read_data(UtFdInputStream *self) {
  // FIXME: Can report more data than requested in a single read - trim buffer
  // in this case.
  ut_list_resize(self->read_buffer, self->read_buffer_length);
  size_t n_used = self->callback(self->user_data, self->read_buffer);
  ut_list_remove(self->read_buffer, 0, n_used);
  self->read_buffer_length -= n_used;
}

static void read_cb(void *user_data) {
  UtFdInputStream *self = user_data;

  if (!ut_cancel_is_active(self->cancel)) {
    // Make space to read a new block.
    ut_list_resize(self->read_buffer,
                   self->read_buffer_length + self->block_size);

    // Read a block.
    ssize_t n_read = read(self->fd,
                          ut_uint8_array_get_data(self->read_buffer) +
                              self->read_buffer_length,
                          self->block_size);
    assert(n_read >= 0);
    self->read_buffer_length += n_read;

    if (n_read == 0) {
      if (self->read_all) {
        report_read_data(self);
      } else {
        UtObjectRef end_of_stream = ut_end_of_stream_new(
            self->read_buffer_length > 0 ? self->read_buffer : NULL);
        if (!ut_cancel_is_active(self->cancel)) {
          self->callback(self->user_data, end_of_stream);
        }
      }
    } else if (!self->read_all) {
      report_read_data(self);
    }
  }

  // Stop listening for read events when done.
  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->watch_cancel);
  }
}

static void add_read_watch(UtFdInputStream *self) {
  ut_event_loop_add_read_watch(self->fd, read_cb, self, self->watch_cancel);
}

static void buffered_read_cb(void *user_data) {
  UtFdInputStream *self = user_data;
  if (ut_cancel_is_active(self->cancel)) {
    return;
  }

  // If have buffered data, process that first.
  if (self->read_buffer_length > 0) {
    report_read_data(self);
    if (self->cancel != NULL && ut_cancel_is_active(self->cancel)) {
      return;
    }
  }

  // Now read more data from the file descriptor.
  add_read_watch(self);
}

static void start_read(UtFdInputStream *self, bool read_all,
                       UtInputStreamCallback callback, void *user_data,
                       UtObject *cancel) {
  // Clean up after the previous read.
  if (self->cancel != NULL && ut_cancel_is_active(self->cancel)) {
    self->read_all = false;
    ut_object_unref(self->watch_cancel);
    self->callback = NULL;
    self->user_data = NULL;
    ut_object_unref(self->cancel);
    self->cancel = NULL;
  }

  assert(self->callback == NULL);

  self->read_all = read_all;
  self->watch_cancel = ut_cancel_new();
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);

  // The read might have been started inside a callback, so wait for that to
  // complete.
  ut_event_loop_add_delay(0, buffered_read_cb, self, cancel);
}

static void ut_fd_input_stream_init(UtObject *object) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  self->fd = -1;
  self->read_buffer = ut_uint8_array_new();
  self->read_buffer_length = 0;
  self->block_size = 4096;
  self->read_all = false;
  self->watch_cancel = NULL;
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
}

static void ut_fd_input_stream_cleanup(UtObject *object) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  ut_object_unref(self->read_buffer);
  ut_object_unref(self->watch_cancel);
  ut_object_unref(self->cancel);
}

static void ut_fd_input_stream_read(UtObject *object,
                                    UtInputStreamCallback callback,
                                    void *user_data, UtObject *cancel) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  assert(self->fd >= 0);
  assert(callback != NULL);

  start_read(self, false, callback, user_data, cancel);
}

static void ut_fd_input_stream_read_all(UtObject *object,
                                        UtInputStreamCallback callback,
                                        void *user_data, UtObject *cancel) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  assert(self->fd >= 0);
  assert(callback != NULL);

  start_read(self, true, callback, user_data, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_fd_input_stream_read, .read_all = ut_fd_input_stream_read_all};

static UtObjectInterface object_interface = {
    .type_name = "UtFdInputStream",
    .init = ut_fd_input_stream_init,
    .cleanup = ut_fd_input_stream_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_fd_input_stream_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtFdInputStream), &object_interface);
  UtFdInputStream *self = (UtFdInputStream *)object;
  self->fd = fd;
  return object;
}

bool ut_object_is_fd_input_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
