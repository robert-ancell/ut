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

typedef struct _WriteBlock WriteBlock;

struct _WriteBlock {
  UtObject *data;
  size_t n_written;
  UtOutputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
  WriteBlock *next;
};

typedef struct {
  UtObject object;
  int fd;
  UtObject *watch_cancel;
  WriteBlock *blocks;
  WriteBlock *last_block;
} UtFdOutputStream;

static void add_block(UtFdOutputStream *self, UtObject *data,
                      UtOutputStreamCallback callback, void *user_data,
                      UtObject *cancel) {
  WriteBlock *block;

  block = malloc(sizeof(WriteBlock));
  block->data = ut_object_ref(data);
  block->n_written = 0;
  block->callback = callback;
  block->user_data = user_data;
  block->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  block->next = NULL;

  if (self->last_block != NULL) {
    self->last_block->next = block;
    self->last_block = block;
  } else {
    self->blocks = self->last_block = block;
  }
}

static void free_block(WriteBlock *block) {
  ut_object_unref(block->data);
  if (block->cancel != NULL) {
    ut_object_unref(block->cancel);
  }
  free(block);
}

static void write_cb(void *user_data) {
  UtFdOutputStream *self = user_data;

  WriteBlock *block = self->blocks;
  assert(block != NULL);

  // Write remaining data.
  size_t block_length = ut_list_get_length(block->data);
  size_t n_to_write = block_length - block->n_written;
  const uint8_t *buffer;
  uint8_t *allocated_buffer = NULL;
  if (ut_object_is_uint8_array(block->data)) {
    buffer = ut_uint8_array_get_data(block->data) + block->n_written;
  } else if (ut_object_is_constant_uint8_array(block->data)) {
    buffer = ut_constant_uint8_array_get_data(block->data) + block->n_written;
  } else {
    allocated_buffer = malloc(sizeof(uint8_t) * n_to_write);
    for (size_t i = 0; i < n_to_write; i++) {
      allocated_buffer[i] =
          ut_uint8_list_get_element(block->data, block->n_written + i);
    }
    buffer = allocated_buffer;
  }
  ssize_t n_written = write(self->fd, buffer, n_to_write);
  free(allocated_buffer);
  assert(n_written >= 0);
  block->n_written += n_written;

  if (block->n_written == block_length) {
    self->blocks = block->next;
    if (self->blocks == NULL) {
      self->last_block = NULL;
    }

    if (block->callback != NULL) {
      block->callback(block->user_data, NULL);
    }

    free_block(block);
  }

  // Stop listening for write events when done.
  if (self->blocks == NULL) {
    ut_cancel_activate(self->watch_cancel);
    ut_object_unref(self->watch_cancel);
    self->watch_cancel = NULL;
  }
}

static void ut_fd_output_stream_init(UtObject *object) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  self->fd = -1;
  self->watch_cancel = NULL;
  self->blocks = NULL;
}

static void ut_fd_output_stream_cleanup(UtObject *object) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  if (self->watch_cancel != NULL) {
    ut_object_unref(self->watch_cancel);
  }
  WriteBlock *next_block;
  for (WriteBlock *b = self->blocks; b != NULL; b = next_block) {
    next_block = b->next;
    free_block(b);
  }
  self->blocks = NULL;
}

static void ut_fd_output_stream_write(UtObject *object, UtObject *data,
                                      UtOutputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  assert(self->fd >= 0);

  add_block(self, data, callback, user_data, cancel);

  if (self->watch_cancel == NULL) {
    self->watch_cancel = ut_cancel_new();
    ut_event_loop_add_write_watch(self->fd, write_cb, self, self->watch_cancel);
  }
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
