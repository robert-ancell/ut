#include <assert.h>
#include <string.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-list.h"
#include "ut-output-stream.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"
#include "ut-unix-domain-socket-client.h"
#include "ut-wayland-client.h"

// https://wayland.freedesktop.org/

// FIXME
#include <stdio.h>

typedef struct {
  UtObject object;
  uint32_t id;
  UtWaylandClientSyncCallback callback;
  void *user_data;
  UtObject *cancel;
} Request;

static void request_cleanup(UtObject *object) {
  Request *self = (Request *)object;
  ut_object_unref(self->cancel);
}

static UtObjectInterface request_object_interface = {
    .type_name = "WaylandRequest",
    .cleanup = request_cleanup,
    .interfaces = {{NULL, NULL}}};

static UtObject *request_new(uint32_t id, UtWaylandClientSyncCallback callback,
                             void *user_data, UtObject *cancel) {
  UtObject *object = ut_object_new(sizeof(Request), &request_object_interface);
  Request *self = (Request *)object;
  self->id = id;
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = ut_object_ref(cancel);
  return object;
}

typedef struct {
  UtObject object;
  UtObject *socket;
  UtObject *read_cancel;

  UtWaylandClientConnectCallback connect_callback;
  void *connect_user_data;
  UtObject *connect_cancel;

  uint32_t next_id;
  uint32_t display_id;
  uint32_t registry_id;
  uint32_t compositor_id;
  uint32_t shm_id;
  uint32_t shell_id;

  UtObject *requests;
} UtWaylandClient;

static uint32_t allocate_id(UtWaylandClient *self) {
  uint32_t id = self->next_id;
  self->next_id++;
  return id;
}

static Request *find_request(UtWaylandClient *self, uint32_t id) {
  size_t requests_length = ut_list_get_length(self->requests);
  for (size_t i = 0; i < requests_length; i++) {
    Request *request = (Request *)ut_object_list_get_element(self->requests, i);
    if (request->id == id) {
      return request;
    }
  }

  return NULL;
}

static void append_uint(UtObject *request, uint32_t value) {
  ut_uint8_list_append_uint32_le(request, value);
}

static void append_int(UtObject *request, int32_t value) {
  append_uint(request, (uint32_t)value);
}

static void append_string(UtObject *request, const char *value) {
  size_t value_length = strlen(value) + 1;
  append_uint(request, value_length);
  for (size_t i = 0; i < value_length; i++) {
    ut_uint8_list_append(request, value[i]);
  }
  for (size_t i = value_length; i % 4 != 0; i++) {
    ut_uint8_list_append(request, 0);
  }
}

static void send_request(UtWaylandClient *self, uint32_t object_id,
                         uint16_t code, UtObject *payload) {
  UtObjectRef request = ut_uint8_array_new();
  uint16_t request_length = 8 + ut_list_get_length(payload);
  append_uint(request, object_id);
  append_uint(request, request_length << 16 | code);
  ut_output_stream_write(self->socket, request);
  ut_output_stream_write(self->socket, payload);
}

static uint32_t display_get_registry(UtWaylandClient *self) {
  uint32_t id = allocate_id(self);
  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, id);
  send_request(self, self->display_id, 1, payload);
  return id;
}

static uint32_t registry_bind(UtWaylandClient *self, uint32_t name,
                              const char *interface, uint32_t version) {
  uint32_t id = allocate_id(self);
  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, name);
  append_string(payload, interface);
  append_uint(payload, version);
  append_uint(payload, id);
  send_request(self, self->registry_id, 0, payload);
  return id;
}

static uint32_t get_uint(UtObject *payload, size_t *offset) {
  // FIXME: Read at native endian
  uint32_t value = ut_uint8_list_get_uint32_le(payload, *offset);
  *offset += 4;
  return value;
}

static char *get_string(UtObject *payload, size_t *offset) {
  size_t payload_length = ut_list_get_length(payload);
  if (payload_length < *offset + 4) {
    return NULL;
  }
  size_t o = *offset;
  uint32_t string_length = get_uint(payload, &o);
  size_t end = o + string_length;
  while (end % 4 != 0) {
    end++;
  }
  if (payload_length < end) {
    return NULL;
  }

  UtObjectRef string = ut_uint8_array_new();
  for (size_t i = 0; i < string_length; i++) {
    ut_uint8_list_append(string, ut_uint8_list_get_element(payload, o));
    o++;
  }
  // FIXME: Validate last character is '\0'

  *offset = end;
  return (char *)ut_uint8_list_take_data(string);
}

static void decode_display_error(UtWaylandClient *self, UtObject *payload) {
  size_t offset = 0;
  uint32_t object_id = get_uint(payload, &offset);
  uint32_t code = get_uint(payload, &offset);
  ut_cstring_ref message = get_string(payload, &offset);

  printf("display error %d %d '%s'\n", object_id, code, message);
}

static void decode_display_delete_id(UtWaylandClient *self, UtObject *payload) {
  size_t offset = 0;
  uint32_t id = get_uint(payload, &offset);
  printf("display delete_id %d\n", id);
}

static void decode_display_event(UtWaylandClient *self, uint16_t code,
                                 UtObject *payload) {
  if (code == 0) {
    decode_display_error(self, payload);
  } else if (code == 1) {
    decode_display_delete_id(self, payload);
  } else {
    assert(false);
  }
}

static void decode_registry_global(UtWaylandClient *self, UtObject *payload) {
  size_t offset = 0;
  uint32_t name = get_uint(payload, &offset);
  ut_cstring_ref interface = get_string(payload, &offset);
  uint32_t version = get_uint(payload, &offset);

  printf("registry global %d %s %d\n", name, interface, version);

  if (strcmp(interface, "wl_compositor") == 0) {
    self->compositor_id = registry_bind(self, name, interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    self->shm_id = registry_bind(self, name, interface, version);
  } else if (strcmp(interface, "wl_shell") == 0) {
    self->shell_id = registry_bind(self, name, interface, version);
  }
}

static void decode_registry_global_remove(UtWaylandClient *self,
                                          UtObject *payload) {
  size_t offset = 0;
  uint32_t name = get_uint(payload, &offset);
  printf("registry global remove %d\n", name);
}

static void decode_registry_event(UtWaylandClient *self, uint16_t code,
                                  UtObject *payload) {
  if (code == 0) {
    decode_registry_global(self, payload);
  } else if (code == 1) {
    decode_registry_global_remove(self, payload);
  } else {
    assert(false);
  }
}

static void decode_shm_format(UtWaylandClient *self, UtObject *payload) {
  size_t offset = 0;
  uint32_t format = get_uint(payload, &offset);
  printf("shm format %08x\n", format);
}

static void decode_shm_event(UtWaylandClient *self, uint16_t code,
                             UtObject *payload) {
  if (code == 0) {
    decode_shm_format(self, payload);
  } else {
    assert(false);
  }
}

static void decode_callback_done(UtWaylandClient *self, Request *request,
                                 UtObject *payload) {
  size_t offset = 0;
  uint32_t callback_data = get_uint(payload, &offset);
  printf("callback done %d\n", callback_data);

  if (request->callback != NULL && !ut_cancel_is_active(request->cancel)) {
    request->callback(request->user_data);
  }
}

static void decode_callback_event(UtWaylandClient *self, Request *request,
                                  uint16_t code, UtObject *payload) {
  if (code == 0) {
    decode_callback_done(self, request, payload);
  } else {
    assert(false);
  }
}

static void decode_event(UtWaylandClient *self, uint32_t object_id,
                         uint16_t code, UtObject *payload) {
  if (object_id == self->display_id) {
    decode_display_event(self, code, payload);
  } else if (object_id == self->registry_id) {
    decode_registry_event(self, code, payload);
  } else if (object_id == self->shm_id) {
    decode_shm_event(self, code, payload);
  } else {
    Request *request = find_request(self, object_id);
    if (request != NULL) {
      decode_callback_event(self, request, code, payload);
    } else {
      printf("%d %d %s\n", object_id, code, ut_object_to_string(payload));
    }
  }
}

static size_t read_cb(void *user_data, UtObject *data, bool complete) {
  UtWaylandClient *self = user_data;

  size_t data_length = ut_list_get_length(data);
  size_t offset = 0;
  while (true) {
    if (data_length < offset + 8) {
      if (complete) {
        // FIXME: Unused data is length > 0
        ut_cancel_activate(self->read_cancel);
      }
      return offset;
    }

    size_t o = offset;
    uint32_t object_id = get_uint(data, &o);
    uint32_t payload_length_and_code = get_uint(data, &o);
    uint16_t payload_length = payload_length_and_code >> 16;
    uint16_t code = payload_length_and_code & 0xffff;

    if (ut_list_get_length(data) < offset + payload_length) {
      if (complete) {
        // FIXME: Missing data
        ut_cancel_activate(self->read_cancel);
      }
      return offset;
    }

    UtObjectRef payload = ut_list_get_sublist(data, o, payload_length - 8);
    decode_event(self, object_id, code, payload);

    offset += payload_length;
  }
}

static void connect_cb(void *user_data) {
  UtWaylandClient *self = user_data;
  if (self->connect_callback != NULL &&
      !ut_cancel_is_active(self->connect_cancel)) {
    self->connect_callback(self->connect_user_data, NULL);
  }
}

static void ut_wayland_client_init(UtObject *object) {
  UtWaylandClient *self = (UtWaylandClient *)object;
  self->socket = NULL;
  self->read_cancel = ut_cancel_new();
  self->connect_callback = NULL;
  self->connect_user_data = NULL;
  self->connect_cancel = NULL;
  self->next_id = 2;
  self->display_id = 1;
  self->registry_id = 0;
  self->compositor_id = 0;
  self->shm_id = 0;
  self->shell_id = 0;
  self->requests = ut_object_list_new();
}

static void ut_wayland_client_cleanup(UtObject *object) {
  UtWaylandClient *self = (UtWaylandClient *)object;
  ut_cancel_activate(self->read_cancel);
  ut_object_unref(self->socket);
  ut_object_unref(self->read_cancel);
  ut_object_unref(self->connect_cancel);
  ut_object_unref(self->requests);
}

static UtObjectInterface object_interface = {.type_name = "UtWaylandClient",
                                             .init = ut_wayland_client_init,
                                             .cleanup =
                                                 ut_wayland_client_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_wayland_client_new() {
  UtObject *object = ut_object_new(sizeof(UtWaylandClient), &object_interface);
  UtWaylandClient *self = (UtWaylandClient *)object;
  self->socket = ut_unix_domain_socket_client_new("/run/user/1000/wayland-0");
  return object;
}

void ut_wayland_client_connect(UtObject *object,
                               UtWaylandClientConnectCallback callback,
                               void *user_data, UtObject *cancel) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  assert(callback != NULL);

  assert(self->connect_callback == NULL);
  self->connect_callback = callback;
  self->connect_user_data = user_data;
  self->connect_cancel = ut_object_ref(cancel);

  ut_unix_domain_socket_client_connect(self->socket);
  ut_input_stream_read(self->socket, read_cb, self, self->read_cancel);

  self->registry_id = display_get_registry(self);
  ut_wayland_client_display_sync(object, connect_cb, self,
                                 NULL); // FIXME: Cancel
}

void ut_wayland_client_display_sync(UtObject *object,
                                    UtWaylandClientSyncCallback callback,
                                    void *user_data, UtObject *cancel) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  uint32_t id = allocate_id(self);
  UtObjectRef request = request_new(id, callback, user_data, cancel);
  ut_list_append(self->requests, request);

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, id);
  send_request(self, self->display_id, 0, payload);
}

uint32_t ut_wayland_client_compositor_create_surface(UtObject *object) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  assert(self->compositor_id != 0);

  uint32_t id = allocate_id(self);

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, id);
  send_request(self, self->compositor_id, 0, payload);

  return id;
}

uint32_t ut_wayland_client_shm_create_pool(UtObject *object, int fd,
                                           size_t size) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  assert(self->shm_id != 0);

  uint32_t id = allocate_id(self);

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, id);
  append_uint(payload, 0); // fd
  append_uint(payload, size);
  send_request(self, self->shm_id, 0, payload);

  return id;
}

uint32_t ut_wayland_client_shell_get_shell_surface(UtObject *object,
                                                   uint32_t surface) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  assert(self->shell_id != 0);

  uint32_t id = allocate_id(self);

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, id);
  append_uint(payload, surface);
  send_request(self, self->shell_id, 0, payload);

  return id;
}

void ut_wayland_client_shell_surface_move(UtObject *object,
                                          uint32_t shell_surface, uint32_t seat,
                                          uint32_t serial) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, seat);
  append_uint(payload, serial);
  send_request(self, shell_surface, 1, payload);
}

void ut_wayland_client_shell_surface_resize(UtObject *object,
                                            uint32_t shell_surface,
                                            uint32_t seat, uint32_t serial,
                                            uint32_t edges) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, seat);
  append_uint(payload, serial);
  append_uint(payload, edges);
  send_request(self, shell_surface, 2, payload);
}

void ut_wayland_client_shell_surface_set_toplevel(UtObject *object,
                                                  uint32_t shell_surface) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  send_request(self, shell_surface, 3, payload);
}

void ut_wayland_client_shell_surface_set_transient(UtObject *object,
                                                   uint32_t shell_surface,
                                                   uint32_t parent, int32_t x,
                                                   int32_t y, uint32_t flags) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, parent);
  append_int(payload, x);
  append_int(payload, y);
  append_uint(payload, flags);
  send_request(self, shell_surface, 4, payload);
}

void ut_wayland_client_shell_surface_set_fullscreen(UtObject *object,
                                                    uint32_t shell_surface,
                                                    uint32_t method,
                                                    uint32_t framerate,
                                                    uint32_t output) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, method);
  append_uint(payload, framerate);
  append_uint(payload, output);
  send_request(self, shell_surface, 5, payload);
}

void ut_wayland_client_shell_surface_set_popup(UtObject *object,
                                               uint32_t shell_surface,
                                               uint32_t seat, uint32_t serial,
                                               uint32_t parent, int32_t x,
                                               int32_t y, uint32_t flags) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, seat);
  append_uint(payload, serial);
  append_uint(payload, parent);
  append_int(payload, x);
  append_int(payload, y);
  append_uint(payload, flags);
  send_request(self, shell_surface, 6, payload);
}

void ut_wayland_client_shell_surface_set_maximized(UtObject *object,
                                                   uint32_t shell_surface,
                                                   uint32_t output) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_uint(payload, output);
  send_request(self, shell_surface, 7, payload);
}

void ut_wayland_client_shell_surface_set_title(UtObject *object,
                                               uint32_t shell_surface,
                                               const char *title) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_string(payload, title);
  send_request(self, shell_surface, 8, payload);
}

void ut_wayland_client_shell_surface_set_class(UtObject *object,
                                               uint32_t shell_surface,
                                               const char *class_) {
  assert(ut_object_is_wayland_client(object));
  UtWaylandClient *self = (UtWaylandClient *)object;

  UtObjectRef payload = ut_uint8_array_new();
  append_string(payload, class_);
  send_request(self, shell_surface, 9, payload);
}

bool ut_object_is_wayland_client(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
