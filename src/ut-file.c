#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-fd-stream.h"
#include "ut-file.h"
#include "ut-list.h"
#include "ut-mutable-list.h"
#include "ut-mutable-uint8-list.h"
#include "ut-object-private.h"
#include "ut-uint8-list.h"

typedef struct {
  char *path;
  int fd;
  UtObject *stream;
} UtFile;

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
  self->stream = NULL;
}

static void ut_file_cleanup(UtObject *object) {
  UtFile *self = ut_object_get_data(object);
  // FIXME: Cancel read/writes
  free(self->path);
  self->path = NULL;
  if (self->stream != NULL) {
    ut_object_unref(self->stream);
    self->stream = NULL;
  }
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
  self->stream = ut_fd_stream_new(self->fd);
}

void ut_file_open_write(UtObject *object, bool create) {
  assert(ut_object_is_file(object));
  UtFile *self = ut_object_get_data(object);
  assert(self->fd == -1);
  int flags = O_TRUNC;
  if (create) {
    flags |= O_CREAT;
  }
  self->fd = open(self->path, O_WRONLY | flags,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  self->stream = ut_fd_stream_new(self->fd);
}

void ut_file_read(UtObject *object, size_t count, UtFileReadCallback callback,
                  void *user_data, UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->stream != NULL);

  ut_fd_stream_read(self->stream, count, callback, user_data, cancel);
}

void ut_file_read_stream(UtObject *object, size_t block_size,
                         UtFileReadCallback callback, void *user_data,
                         UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->stream != NULL);

  ut_fd_stream_read_stream(self->stream, block_size, callback, user_data,
                           cancel);
}

void ut_file_read_all(UtObject *object, size_t block_size,
                      UtFileReadCallback callback, void *user_data,
                      UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->stream != NULL);

  ut_fd_stream_read_all(self->stream, block_size, callback, user_data, cancel);
}

void ut_file_write(UtObject *object, UtObject *data,
                   UtFileWriteCallback callback, void *user_data,
                   UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->stream != NULL);

  ut_fd_stream_write(self->stream, data, callback, user_data, cancel);
}

void ut_file_write_all(UtObject *object, UtObject *data,
                       UtFileWriteCallback callback, void *user_data,
                       UtObject *cancel) {
  UtFile *self = ut_object_get_data(object);
  assert(self->stream != NULL);

  ut_fd_stream_write_all(self->stream, data, callback, user_data, cancel);
}

void ut_file_close(UtObject *object) {
  assert(ut_object_is_file(object));
  UtFile *self = ut_object_get_data(object);
  close_file(self);
}

bool ut_object_is_file(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
