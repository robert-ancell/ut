#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ut-cancel.h"
#include "ut-event-loop.h"
#include "ut-fd-stream.h"
#include "ut-object-private.h"
#include "ut-tcp-client.h"

typedef struct {
  UtObject object;
  char *address;
  uint16_t port;
  int fd;
  bool connecting;
  bool connected;
  UtObject *stream;
} UtTcpClient;

typedef struct {
  UtTcpClient *self;
  char *address;
  int port;
  UtObject *watch_cancel;
  UtTcpClientConnectCallback callback;
  void *user_data;
  UtObject *cancel;
} ConnectData;

static ConnectData *connect_data_new(UtTcpClient *self, const char *address,
                                     int port,
                                     UtTcpClientConnectCallback callback,
                                     void *user_data, UtObject *cancel) {
  ConnectData *data = malloc(sizeof(ConnectData));
  data->self = self;
  data->address = strdup(address);
  data->port = port;
  data->watch_cancel = ut_cancel_new();
  data->callback = callback;
  data->user_data = user_data;
  data->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;
  return data;
}

static void connect_data_free(ConnectData *data) {
  free(data->address);
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

  self->stream = ut_fd_stream_new(self->fd);

  int error; // FIXME: use
  getsockopt(self->fd, SOL_SOCKET, SO_ERROR, &error, NULL);
  assert(error == 0);

  bool is_cancelled = data->cancel != NULL && ut_cancel_is_active(data->cancel);
  if (!is_cancelled && data->callback != NULL) {
    data->callback(data->user_data);
  }

  ut_cancel_activate(data->watch_cancel);
  connect_data_free(data);
}

static void *lookup_thread_cb(void *data_) {
  ConnectData *data = data_;

  char port_string[6];
  snprintf(port_string, 6, "%d", data->port);
  struct addrinfo *addresses;
  getaddrinfo(data->address, port_string, NULL, &addresses);
  return addresses;
}

static void lookup_result_cb(void *user_data, void *result) {
  ConnectData *data = user_data;
  UtTcpClient *self = data->self;
  struct addrinfo *addresses = result;

  assert(addresses != NULL);
  struct addrinfo *address = addresses;

  self->fd = socket(address->ai_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(self->fd >= 0);

  ut_event_loop_add_write_watch(self->fd, connect_cb, data, data->watch_cancel);

  int connect_result = connect(self->fd, address->ai_addr, address->ai_addrlen);
  assert(connect_result == 0 || errno == EINPROGRESS);

  freeaddrinfo(addresses);
}

static void disconnect_client(UtTcpClient *self) {
  if (self->fd >= 0) {
    close(self->fd);
    self->fd = -1;
  }
}

static void ut_tcp_client_init(UtObject *object) {
  UtTcpClient *self = (UtTcpClient *)object;
  self->address = NULL;
  self->port = 0;
  self->fd = -1;
  self->connecting = false;
  self->connected = false;
  self->stream = NULL;
}

static void ut_tcp_client_cleanup(UtObject *object) {
  UtTcpClient *self = (UtTcpClient *)object;
  free(self->address);
  disconnect_client(self);
}

static UtObjectFunctions object_functions = {.init = ut_tcp_client_init,
                                             .cleanup = ut_tcp_client_cleanup};

UtObject *ut_tcp_client_new(const char *address, uint16_t port) {
  UtObject *object = ut_object_new(sizeof(UtTcpClient), &object_functions);
  UtTcpClient *self = (UtTcpClient *)object;
  self->address = strdup(address);
  self->port = port;
  return object;
}

void ut_tcp_client_connect(UtObject *object,
                           UtTcpClientConnectCallback callback, void *user_data,
                           UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(!self->connecting);
  self->connecting = true;

  // Lookup address.
  // FIXME: Cancel thread if this UtTcpClient is deleted.
  ConnectData *data = connect_data_new(self, self->address, self->port,
                                       callback, user_data, cancel);
  ut_event_loop_run_in_thread(lookup_thread_cb, data, NULL, lookup_result_cb,
                              data, NULL);
}

void ut_tcp_client_read(UtObject *object, size_t count,
                        UtTcpClientReadCallback callback, void *user_data,
                        UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->stream != NULL);

  ut_fd_stream_read(self->stream, count, callback, user_data, cancel);
}

void ut_tcp_client_read_all(UtObject *object, UtTcpClientReadCallback callback,
                            void *user_data, UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->stream != NULL);

  ut_fd_stream_read_all(self->stream, 65535, callback, user_data, cancel);
}

void ut_tcp_client_write(UtObject *object, UtObject *data,
                         UtTcpClientWriteCallback callback, void *user_data,
                         UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->stream != NULL);

  ut_fd_stream_write(self->stream, data, callback, user_data, cancel);
}

void ut_tcp_client_write_all(UtObject *object, UtObject *data,
                             UtTcpClientWriteCallback callback, void *user_data,
                             UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->stream != NULL);

  ut_fd_stream_write_all(self->stream, data, callback, user_data, cancel);
}

void ut_tcp_client_disconnect(UtObject *object) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  disconnect_client(self);
}

bool ut_object_is_tcp_client(UtObject *object) {
  return ut_object_is_type(object, &object_functions);
}
