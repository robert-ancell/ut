#include <assert.h>
#include <unistd.h>

#include "ut-cstring.h"
#include "ut-file-descriptor.h"

typedef struct {
  UtObject object;
  int fd;
} UtFileDescriptor;

static void ut_file_descriptor_init(UtObject *object) {
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  self->fd = -1;
}

static char *ut_file_descriptor_to_string(UtObject *object) {
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  return ut_cstring_new_printf("<UtFileDescriptor>(%d)", self->fd);
}

static void ut_file_descriptor_cleanup(UtObject *object) {
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  if (self->fd >= 0) {
    close(self->fd);
  }
}

static UtObjectInterface object_interface = {
    .type_name = "UtFileDescriptor",
    .init = ut_file_descriptor_init,
    .to_string = ut_file_descriptor_to_string,
    .cleanup = ut_file_descriptor_cleanup,
    .interfaces = {{NULL, NULL}}};

UtObject *ut_file_descriptor_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtFileDescriptor), &object_interface);
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  self->fd = fd;
  return object;
}

int ut_file_descriptor_get_fd(UtObject *object) {
  assert(ut_object_is_file_descriptor(object));
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  return self->fd;
}

int ut_file_descriptor_take_fd(UtObject *object) {
  assert(ut_object_is_file_descriptor(object));
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  int fd = self->fd;
  self->fd = -1;
  return fd;
}

void ut_file_descriptor_close(UtObject *object) {
  assert(ut_object_is_file_descriptor(object));
  UtFileDescriptor *self = (UtFileDescriptor *)object;
  close(self->fd);
  self->fd = -1;
}

bool ut_object_is_file_descriptor(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
