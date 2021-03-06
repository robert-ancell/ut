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
#include "ut-fd-input-stream.h"
#include "ut-fd-output-stream.h"
#include "ut-file-descriptor.h"
#include "ut-input-stream.h"
#include "ut-output-stream.h"
#include "ut-tcp-client.h"

typedef struct {
  UtObject object;
  char *address;
  uint16_t port;
  UtObject *fd;
  bool connecting;
  bool connected;
  UtObject *input_stream;
  UtObject *output_stream;
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
  data->cancel = ut_object_ref(cancel);
  return data;
}

static void connect_data_free(ConnectData *data) {
  free(data->address);
  ut_object_unref(data->watch_cancel);
  ut_object_unref(data->cancel);
  free(data);
}

static void connect_cb(void *user_data) {
  ConnectData *data = user_data;
  UtTcpClient *self = data->self;

  self->connected = true;

  self->input_stream = ut_fd_input_stream_new(self->fd);
  self->output_stream = ut_fd_output_stream_new(self->fd);

  int error; // FIXME: use
  socklen_t error_length = sizeof(error);
  getsockopt(ut_file_descriptor_get_fd(self->fd), SOL_SOCKET, SO_ERROR, &error,
             &error_length);
  assert(error == 0);

  if (!ut_cancel_is_active(data->cancel) && data->callback != NULL) {
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

  int fd = socket(address->ai_family, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(fd >= 0);
  self->fd = ut_file_descriptor_new(fd);

  ut_event_loop_add_write_watch(self->fd, connect_cb, data, data->watch_cancel);

  int connect_result = connect(fd, address->ai_addr, address->ai_addrlen);
  assert(connect_result == 0 || errno == EINPROGRESS);

  freeaddrinfo(addresses);
}

static void disconnect_client(UtTcpClient *self) {
  if (self->fd != NULL) {
    ut_object_unref(self->fd);
    self->fd = NULL;
  }
}

static void ut_tcp_client_init(UtObject *object) {
  UtTcpClient *self = (UtTcpClient *)object;
  self->address = NULL;
  self->port = 0;
  self->fd = NULL;
  self->connecting = false;
  self->connected = false;
  self->input_stream = NULL;
  self->output_stream = NULL;
}

static void ut_tcp_client_cleanup(UtObject *object) {
  UtTcpClient *self = (UtTcpClient *)object;
  free(self->address);
  ut_object_unref(self->fd);
  ut_object_unref(self->input_stream);
  ut_object_unref(self->output_stream);
  disconnect_client(self);
}

static void ut_tcp_client_read(UtObject *object, UtInputStreamCallback callback,
                               void *user_data, UtObject *cancel) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->input_stream != NULL);

  ut_input_stream_read(self->input_stream, callback, user_data, cancel);
}

static void ut_tcp_client_set_active(UtObject *object, bool active) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->input_stream != NULL);

  ut_input_stream_set_active(self->input_stream, active);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_tcp_client_read, .set_active = ut_tcp_client_set_active};

static void ut_tcp_client_write(UtObject *object, UtObject *data,
                                UtOutputStreamCallback callback,
                                void *user_data, UtObject *cancel) {
  UtTcpClient *self = (UtTcpClient *)object;
  assert(self->output_stream != NULL);

  ut_output_stream_write_full(self->output_stream, data, callback, user_data,
                              cancel);
}

static UtOutputStreamInterface output_stream_interface = {
    .write = ut_tcp_client_write};

static UtObjectInterface object_interface = {
    .type_name = "UtTcpClient",
    .init = ut_tcp_client_init,
    .cleanup = ut_tcp_client_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {&ut_output_stream_id, &output_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_tcp_client_new(const char *address, uint16_t port) {
  UtObject *object = ut_object_new(sizeof(UtTcpClient), &object_interface);
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
  ut_event_loop_add_worker_thread(lookup_thread_cb, data, NULL,
                                  lookup_result_cb, data, NULL);
}

void ut_tcp_client_disconnect(UtObject *object) {
  assert(ut_object_is_tcp_client(object));
  UtTcpClient *self = (UtTcpClient *)object;
  disconnect_client(self);
}

bool ut_object_is_tcp_client(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
