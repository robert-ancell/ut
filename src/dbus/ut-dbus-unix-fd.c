#include <assert.h>
#include <unistd.h>

#include "ut-cstring.h"
#include "ut-dbus-unix-fd.h"

typedef struct {
  UtObject object;
  int fd;
} UtDBusUnixFd;

static void ut_dbus_unix_fd_init(UtObject *object) {
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  self->fd = -1;
}

static char *ut_dbus_unix_fd_to_string(UtObject *object) {
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  return ut_cstring_new_printf("<UtDBusUnixFd>(%d)", self->fd);
}

static void ut_dbus_unix_fd_cleanup(UtObject *object) {
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  if (self->fd >= 0) {
    close(self->fd);
  }
}

static UtObjectInterface object_interface = {.type_name = "UtDBusUnixFd",
                                             .init = ut_dbus_unix_fd_init,
                                             .to_string =
                                                 ut_dbus_unix_fd_to_string,
                                             .cleanup = ut_dbus_unix_fd_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_dbus_unix_fd_new(int fd) {
  UtObject *object = ut_object_new(sizeof(UtDBusUnixFd), &object_interface);
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  self->fd = fd;
  return object;
}

int ut_dbus_unix_fd_get_fd(UtObject *object) {
  assert(ut_object_is_dbus_unix_fd(object));
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  return self->fd;
}

int ut_dbus_unix_fd_take_fd(UtObject *object) {
  assert(ut_object_is_dbus_unix_fd(object));
  UtDBusUnixFd *self = (UtDBusUnixFd *)object;
  int fd = self->fd;
  self->fd = -1;
  return fd;
}

bool ut_object_is_dbus_unix_fd(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
