#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-object-private.h"
#include "ut-tcp-client.h"

typedef struct {
  int domain;
  char *address;
  uint16_t port;
  int fd;
  bool connected;
} UtTcpClient;

typedef struct {
  UtTcpClient *self;
  UtObject *watch_cancel;
  UtTcpClientConnectCallback callback;
  void *user_data;
  UtObject *cancel;
} ConnectData;

static ConnectData *connect_data_new(UtTcpClient *self,
                                     UtTcpClientConnectCallback callback,
                                     void *user_data, UtObject *cancel) {
  ConnectData *data = malloc(sizeof(ConnectData));
  data->self = self;
  data->watch_cancel = ut_cancel_new();
  data->callback = callback;
  data->user_data = user_data;
  data->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  return data;
}

static void connect_data_free(ConnectData *data) {
  ut_object_unref(data->watch_cancel);
  if (data->cancel) {
    ut_object_unref(data->cancel);
  }
  free(data);
}

static void connect_cb(void *user_data) {
  ConnectData *data = user_data;
  UtTcpClient *self = data->self;

  self->connected = true;

  int error; // FIXME: use
  getsockopt(self->fd, SOL_SOCKET, SO_ERROR, &error, NULL);
  if (data->callback != NULL) {
    data->callback(data->user_data);
  }

  ut_cancel_activate(data->watch_cancel);
  connect_data_free(data);
}

static void disconnect_client(UtTcpClient *self) {
  if (self->fd >= 0) {
    close(self->fd);
    self->fd = -1;
  }
}

static void ut_tcp_client_init(UtObject *object) {
  UtTcpClient *self = ut_object_get_data(object);
  self->domain = 0;
  self->address = NULL;
  self->port = 0;
  self->fd = -1;
  self->connected = false;
}

static void ut_tcp_client_cleanup(UtObject *object) {
  UtTcpClient *self = ut_object_get_data(object);
  free(self->address);
  disconnect_client(self);
}

static UtObjectFunctions object_functions = {.init = ut_tcp_client_init,
                                             .cleanup = ut_tcp_client_cleanup};

UtObject *ut_tcp_client_new(const char *address, uint16_t port) {
  UtObject *object = ut_object_new(sizeof(UtTcpClient), &object_functions);
  UtTcpClient *self = ut_object_get_data(object);
  self->domain = AF_INET; // FIXME AF_INET6
  self->address = strdup(address);
  self->port = port;
  return object;
}

void ut_tcp_client_connect(UtObject *object,
                           UtTcpClientConnectCallback callback, void *user_data,
                           UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = ut_object_get_data(object);
  assert(self->fd < 0);
  self->fd = socket(self->domain, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(self->fd >= 0);

  ConnectData *data = connect_data_new(self, callback, user_data, cancel);
  ut_event_loop_add_write_watch(self->fd, connect_cb, data, data->watch_cancel);

  struct sockaddr_in addr;
  addr.sin_family = self->domain;
  addr.sin_port = htons(self->port);
  inet_pton(self->domain, self->address, &addr.sin_addr);
  int result = connect(self->fd, (struct sockaddr *)&addr, sizeof(addr));
  assert(result == 0 || errno == EINPROGRESS);
}

void ut_tcp_client_disconnect(UtObject *object) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = ut_object_get_data(object);
  disconnect_client(self);
}

bool ut_object_is_tcp_client(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
