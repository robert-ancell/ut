#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-end-of-stream.h"
#include "ut-event-loop.h"
#include "ut-fd-output-stream.h"
#include "ut-list.h"
#include "ut-object-private.h"
#include "ut-output-stream.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  int fd;
  UtObject *data;
  size_t n_written;
  UtObject *watch_cancel;
  UtOutputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} UtFdOutputStream;

static void write_cb(void *user_data) {
  UtFdOutputStream *self = user_data;

  bool done = false;
  if (self->cancel == NULL || !ut_cancel_is_active(self->cancel)) {
    // Write remaining data.
    size_t n_to_write = ut_list_get_length(self->data) - self->n_written;
    const uint8_t *buffer;
    uint8_t *allocated_buffer = NULL;
    if (ut_object_is_uint8_array(self->data)) {
      buffer = ut_uint8_array_get_data(self->data) + self->n_written;
    } else if (ut_object_is_constant_uint8_array(self->data)) {
      buffer = ut_constant_uint8_array_get_data(self->data) + self->n_written;
    } else {
      allocated_buffer = malloc(sizeof(uint8_t) * n_to_write);
      for (size_t i = 0; i < n_to_write; i++) {
        allocated_buffer[i] =
            ut_uint8_list_get_element(self->data, self->n_written + i);
      }
    }
    ssize_t n_written = write(self->fd, buffer, n_to_write);
    if (allocated_buffer != NULL) {
      free(allocated_buffer);
    }
    assert(n_written >= 0);
    self->n_written += n_written;

    // Done if all data written or only doing single write.
    done = self->n_written == ut_list_get_length(self->data);

    // Report how much data was written.
    if (done && self->callback != NULL) {
      self->callback(self->user_data, NULL);
    }
  } else {
    done = true;
  }

  // Stop listening for write events when done.
  if (done) {
    ut_cancel_activate(self->watch_cancel);
    ut_object_unref(self->data);
    self->data = NULL;
    self->n_written = 0;
    ut_object_unref(self->watch_cancel);
    self->watch_cancel = NULL;
    self->callback = NULL;
    self->user_data = NULL;
    if (self->cancel != NULL) {
      ut_object_unref(self->cancel);
      self->cancel = NULL;
    }
  }
}

static void ut_fd_output_stream_init(UtObject *object) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  self->fd = -1;
  self->data = NULL;
  self->n_written = 0;
  self->watch_cancel = NULL;
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
}

static void ut_fd_output_stream_cleanup(UtObject *object) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  if (self->data != NULL) {
    ut_object_unref(self->data);
  }
  if (self->watch_cancel != NULL) {
    ut_object_unref(self->watch_cancel);
  }
  if (self->cancel != NULL) {
    ut_object_unref(self->cancel);
  }
}

static void ut_fd_output_stream_write(UtObject *object, UtObject *data,
                                      UtOutputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  assert(self->fd >= 0);

  self->data = ut_object_ref(data);
  self->n_written = 0;
  self->watch_cancel = ut_cancel_new();
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  ut_event_loop_add_write_watch(self->fd, write_cb, self, self->watch_cancel);
}

static UtOutputStreamInterface output_stream_interface = {
    .write = ut_fd_output_stream_write};

static UtObjectInterface object_interface = {
    .type_name = "UtFdOutputStream",
    .init = ut_fd_output_stream_init,
    .cleanup = ut_fd_output_stream_cleanup,
    .interfaces = {{&ut_output_stream_id, &output_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_fd_output_stream_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtFdOutputStream), &object_interface);
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  self->fd = fd;
  return object;
}

bool ut_object_is_fd_output_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
