#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-event-loop.h"
#include "ut-fd-input-stream.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  int fd;
  UtObject *read_buffer;
  bool active;
  bool complete;
  size_t block_size;
  UtObject *watch_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} UtFdInputStream;

static void read_cb(void *user_data) {
  UtFdInputStream *self = user_data;

  // Stop listening for read events when consumer no longer wants them.
  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->watch_cancel);
    return;
  }

  // Make space to read a new block.
  size_t buffer_length = ut_list_get_length(self->read_buffer);
  ut_list_resize(self->read_buffer, buffer_length + self->block_size);

  // Read a block.
  ssize_t n_read =
      read(self->fd, ut_uint8_array_get_data(self->read_buffer) + buffer_length,
           self->block_size);
  assert(n_read >= 0);
  buffer_length += n_read;
  ut_list_resize(self->read_buffer, buffer_length);

  // No more data to read.
  if (n_read == 0) {
    ut_cancel_activate(self->watch_cancel);
    self->complete = true;
  }

  size_t n_used =
      self->callback(self->user_data, self->read_buffer, self->complete);
  ut_list_remove(self->read_buffer, 0, n_used);

  // Stop listening for read events when consumer no longer wants them.
  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->watch_cancel);
  }
}

static void ut_fd_input_stream_init(UtObject *object) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  self->fd = -1;
  self->read_buffer = ut_uint8_array_new();
  self->active = false;
  self->complete = false;
  self->block_size = 4096;
  self->watch_cancel = ut_cancel_new();
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
}

static void ut_fd_input_stream_cleanup(UtObject *object) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  ut_object_unref(self->read_buffer);
  if (self->watch_cancel != NULL) {
    ut_cancel_activate(self->watch_cancel);
  }
  ut_object_unref(self->watch_cancel);
  ut_object_unref(self->cancel);
}

static void ut_fd_input_stream_read(UtObject *object,
                                    UtInputStreamCallback callback,
                                    void *user_data, UtObject *cancel) {
  UtFdInputStream *self = (UtFdInputStream *)object;
  assert(self->fd >= 0);
  assert(callback != NULL);

  assert(self->callback == NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);

  self->active = true;
  ut_event_loop_add_read_watch(self->fd, read_cb, self, self->watch_cancel);
}

static void ut_fd_input_stream_set_active(UtObject *object, bool active) {
  UtFdInputStream *self = (UtFdInputStream *)object;

  if (self->active == !!active) {
    return;
  }
  self->active = !!active;

  if (self->callback == NULL) {
    return;
  }

  if (active) {
    ut_event_loop_add_read_watch(self->fd, read_cb, self, self->watch_cancel);
    if (ut_list_get_length(self->read_buffer) > 0) {
      size_t n_used =
          self->callback(self->user_data, self->read_buffer, self->complete);
      ut_list_remove(self->read_buffer, 0, n_used);
    }
  } else {
    ut_cancel_activate(self->watch_cancel);
    ut_object_unref(self->watch_cancel);
    self->watch_cancel = ut_cancel_new();
  }
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_fd_input_stream_read,
    .set_active = ut_fd_input_stream_set_active};

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
