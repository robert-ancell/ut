#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-constant-uint8-array.h"
#include "ut-event-loop.h"
#include "ut-fd-output-stream.h"
#include "ut-file-descriptor.h"
#include "ut-list.h"
#include "ut-output-stream.h"
#include "ut-uint8-array-with-fds.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct _WriteBlock WriteBlock;

struct _WriteBlock {
  UtObject *data;
  size_t n_written;
  bool sent_fds;
  UtOutputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
  WriteBlock *next;
};

typedef struct {
  UtObject object;
  UtObject *fd;
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
  block->sent_fds = false;
  block->callback = callback;
  block->user_data = user_data;
  block->cancel = ut_object_ref(cancel);
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
  ut_object_unref(block->cancel);
  free(block);
}

static void write_cb(void *user_data) {
  UtFdOutputStream *self = user_data;

  WriteBlock *block = self->blocks;
  assert(block != NULL);

  // Write remaining data.
  size_t block_length = ut_list_get_length(block->data);
  size_t n_to_write = block_length - block->n_written;
  UtObject *data;
  UtObject *file_descriptors = NULL;
  if (ut_object_is_uint8_array_with_fds(block->data)) {
    data = ut_uint8_array_with_fds_get_data(block->data);
    file_descriptors = ut_uint8_array_with_fds_get_fds(block->data);
  } else {
    data = block->data;
  }
  const uint8_t *buffer;
  uint8_t *allocated_buffer = NULL;
  if (ut_object_is_uint8_array(data)) {
    buffer = ut_uint8_array_get_data(data) + block->n_written;
  } else if (ut_object_is_constant_uint8_array(data)) {
    buffer = ut_constant_uint8_array_get_data(data) + block->n_written;
  } else {
    allocated_buffer = malloc(sizeof(uint8_t) * n_to_write);
    for (size_t i = 0; i < n_to_write; i++) {
      allocated_buffer[i] =
          ut_uint8_list_get_element(data, block->n_written + i);
    }
    buffer = allocated_buffer;
  }
  ssize_t n_written;
  if (!block->sent_fds && file_descriptors != NULL &&
      ut_list_get_length(file_descriptors) > 0) {
    size_t file_descriptors_length = ut_list_get_length(file_descriptors);
    struct iovec iov;
    iov.iov_base = (void *)buffer;
    iov.iov_len = n_to_write;
    uint8_t control_data[CMSG_SPACE(sizeof(int) * file_descriptors_length)];
    struct msghdr msg;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_data;
    msg.msg_controllen = sizeof(control_data);
    msg.msg_flags = 0;
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int) * file_descriptors_length);
    int cmsg_fds[file_descriptors_length];
    for (size_t i = 0; i < file_descriptors_length; i++) {
      UtObjectRef fd = ut_list_get_element(file_descriptors, i);
      cmsg_fds[i] = ut_file_descriptor_get_fd(fd);
    }
    memcpy(CMSG_DATA(cmsg), cmsg_fds, sizeof(cmsg_fds));
    n_written = sendmsg(ut_file_descriptor_get_fd(self->fd), &msg, 0);
    if (n_written >= 0) {
      block->sent_fds = true;
    }
  } else {
    n_written = write(ut_file_descriptor_get_fd(self->fd), buffer, n_to_write);
  }
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
  self->fd = NULL;
  self->watch_cancel = NULL;
  self->blocks = NULL;
  self->last_block = NULL;
}

static void ut_fd_output_stream_cleanup(UtObject *object) {
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  ut_object_unref(self->fd);
  ut_object_unref(self->watch_cancel);
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

UtObject *ut_fd_output_stream_new(UtObject *fd) {
  UtObject *object = ut_object_new(sizeof(UtFdOutputStream), &object_interface);
  UtFdOutputStream *self = (UtFdOutputStream *)object;
  self->fd = ut_object_ref(fd);
  return object;
}

bool ut_object_is_fd_output_stream(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
