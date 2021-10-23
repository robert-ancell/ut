#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-fd-stream.h"
#include "ut-file.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-local-file.h"
#include "ut-mutable-list.h"
#include "ut-object-private.h"
#include "ut-output-stream.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
  char *path;
  int fd;
  UtObject *stream;
} UtLocalFile;

static void close_file(UtLocalFile *self) {
  if (self->fd >= 0) {
    close(self->fd);
    self->fd = -1;
  }
}

static void ut_local_file_init(UtObject *object) {
  UtLocalFile *self = (UtLocalFile *)object;
  self->path = NULL;
  self->fd = -1;
  self->stream = NULL;
}

static void ut_local_file_cleanup(UtObject *object) {
  UtLocalFile *self = (UtLocalFile *)object;
  // FIXME: Cancel read/writes
  free(self->path);
  self->path = NULL;
  if (self->stream != NULL) {
    ut_object_unref(self->stream);
    self->stream = NULL;
  }
  close_file(self);
}

static void ut_local_file_open_read(UtObject *object) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->fd == -1);
  self->fd = open(self->path, O_RDONLY);
  self->stream = ut_fd_stream_new(self->fd);
}

static void ut_local_file_open_write(UtObject *object, bool create) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->fd == -1);
  int flags = O_TRUNC;
  if (create) {
    flags |= O_CREAT;
  }
  self->fd = open(self->path, O_WRONLY | flags,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  self->stream = ut_fd_stream_new(self->fd);
}

static void ut_local_file_close(UtObject *object) {
  UtLocalFile *self = (UtLocalFile *)object;
  close_file(self);
}

static void ut_local_file_read(UtObject *object, size_t block_size,
                               UtInputStreamCallback callback, void *user_data,
                               UtObject *cancel) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->stream != NULL);

  ut_input_stream_read(self->stream, block_size, callback, user_data, cancel);
}

static void ut_local_file_read_all(UtObject *object, size_t block_size,
                                   UtInputStreamCallback callback,
                                   void *user_data, UtObject *cancel) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->stream != NULL);

  ut_input_stream_read_all(self->stream, block_size, callback, user_data,
                           cancel);
}

static void ut_local_file_write(UtObject *object, UtObject *data,
                                UtOutputStreamCallback callback,
                                void *user_data, UtObject *cancel) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->stream != NULL);

  ut_output_stream_write(self->stream, data, callback, user_data, cancel);
}

static void ut_local_file_write_all(UtObject *object, UtObject *data,
                                    UtOutputStreamCallback callback,
                                    void *user_data, UtObject *cancel) {
  UtLocalFile *self = (UtLocalFile *)object;
  assert(self->stream != NULL);

  ut_output_stream_write_all(self->stream, data, callback, user_data, cancel);
}

static UtFileInterface file_interface = {.open_read = ut_local_file_open_read,
                                         .open_write = ut_local_file_open_write,
                                         .close = ut_local_file_close};

static UtInputStreamInterface input_stream_interface = {
    .read = ut_local_file_read, .read_all = ut_local_file_read_all};

static UtOutputStreamInterface output_stream_interface = {
    .write = ut_local_file_write, .write_all = ut_local_file_write_all};

static UtObjectInterface object_interface = {
    .type_name = "UtLocalFile",
    .init = ut_local_file_init,
    .cleanup = ut_local_file_cleanup,
    .interfaces = {{&ut_file_id, &file_interface},
                   {&ut_input_stream_id, &input_stream_interface},
                   {&ut_output_stream_id, &output_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_local_file_new(const char *path) {
  UtObject *object = ut_object_new(sizeof(UtLocalFile), &object_interface);
  UtLocalFile *self = (UtLocalFile *)object;
  self->path = strdup(path);
  return object;
}

bool ut_object_is_file(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
