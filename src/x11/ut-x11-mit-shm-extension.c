#include <assert.h>
#include <unistd.h>

#include "ut-x11-buffer.h"
#include "ut-x11-client-private.h"
#include "ut-x11-extension.h"
#include "ut-x11-mit-shm-extension.h"
#include "ut-x11-shm-segment-error.h"

typedef struct _UtX11MitShmExtension UtX11MitShmExtension;

typedef struct {
  UtObject object;
  UtX11MitShmExtension *self;
  void *callback;
  void *user_data;
} CallbackData;

static UtObjectInterface callback_data_object_interface = {
    .type_name = "CallbackData", .interfaces = {{NULL, NULL}}};

static UtObject *callback_data_new(UtX11MitShmExtension *self, void *callback,
                                   void *user_data) {
  UtObject *object =
      ut_object_new(sizeof(CallbackData), &callback_data_object_interface);
  CallbackData *data = (CallbackData *)object;
  data->self = self;
  data->callback = callback;
  data->user_data = user_data;
  return object;
}

struct _UtX11MitShmExtension {
  UtObject object;
  UtObject *client;
  uint8_t major_opcode;
  uint8_t first_event;
  uint8_t first_error;
  uint16_t uid;
  uint16_t gid;
  uint8_t pixmap_format;
  bool shared_pixmaps;
};

static void ut_x11_mit_shm_extension_init(UtObject *object) {
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  self->major_opcode = 0;
  self->first_event = 0;
  self->first_error = 0;
  self->uid = 0;
  self->gid = 0;
  self->pixmap_format = 0;
  self->shared_pixmaps = 0;
}

static void decode_shm_query_version_reply(UtObject *user_data, uint8_t data0,
                                           UtObject *data) {
  CallbackData *callback_data = (CallbackData *)user_data;
  UtX11MitShmExtension *self = callback_data->self;

  size_t offset = 0;
  self->shared_pixmaps = data0 != 0;
  /*uint16_t major_version = */ ut_x11_buffer_get_card16(data, &offset);
  /*uint16_t minor_version = */ ut_x11_buffer_get_card16(data, &offset);
  self->uid = ut_x11_buffer_get_card16(data, &offset);
  self->gid = ut_x11_buffer_get_card16(data, &offset);
  self->pixmap_format = ut_x11_buffer_get_card8(data, &offset);
  ut_x11_buffer_get_padding(data, &offset, 15);

  if (callback_data->callback != NULL) {
    UtX11MitShmEnableCallback callback =
        (UtX11MitShmEnableCallback)callback_data->callback;
    callback(callback_data->user_data, NULL);
  }
}

static void handle_shm_query_version_error(UtObject *user_data,
                                           UtObject *error) {
  CallbackData *callback_data = (CallbackData *)user_data;

  if (callback_data->callback != NULL) {
    UtX11MitShmEnableCallback callback =
        (UtX11MitShmEnableCallback)callback_data->callback;
    callback(callback_data->user_data, error);
  }
}

static void decode_shm_create_segment_reply(UtObject *user_data, uint8_t data0,
                                            UtObject *data) {
  CallbackData *callback_data = (CallbackData *)user_data;

  size_t fds_length = data0;
  assert(ut_x11_buffer_get_fd_count(data) >= fds_length);
  UtObjectRef fd = ut_x11_buffer_take_fd(data);

  if (callback_data->callback != NULL) {
    UtX11MitShmCreateSegmentCallback callback =
        (UtX11MitShmCreateSegmentCallback)callback_data->callback;
    callback(callback_data->user_data, fd, NULL);
  }
}

static void handle_shm_create_segment_error(UtObject *user_data,
                                            UtObject *error) {
  CallbackData *callback_data = (CallbackData *)user_data;

  if (callback_data->callback != NULL) {
    UtX11MitShmCreateSegmentCallback callback =
        (UtX11MitShmCreateSegmentCallback)callback_data->callback;
    callback(callback_data->user_data, NULL, error);
  }
}

static UtObject *ut_x11_mit_shm_extension_decode_event(UtObject *object,
                                                       UtObject *data) {
  return NULL;
}

static UtObject *ut_x11_mit_shm_extension_decode_error(UtObject *object,
                                                       UtObject *data) {
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  size_t offset = 0;
  assert(ut_x11_buffer_get_card8(data, &offset) == 0);
  uint8_t code = ut_x11_buffer_get_card8(data, &offset);
  /*uint16_t sequence_number = */ ut_x11_buffer_get_card16(data, &offset);
  uint32_t value = ut_x11_buffer_get_card32(data, &offset);

  if (code == self->first_error) {
    return ut_x11_shm_segment_error_new(value);
  }

  return NULL;
}

static void ut_x11_mit_shm_extension_close(UtObject *object) {
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  self->client = NULL;
}

static UtX11ExtensionInterface x11_extension_interface = {
    .decode_event = ut_x11_mit_shm_extension_decode_event,
    .decode_error = ut_x11_mit_shm_extension_decode_error,
    .close = ut_x11_mit_shm_extension_close};

static UtObjectInterface object_interface = {
    .type_name = "UtX11MitShmExtension",
    .init = ut_x11_mit_shm_extension_init,
    .interfaces = {{&ut_x11_extension_id, &x11_extension_interface},
                   {NULL, NULL}}};

UtObject *ut_x11_mit_shm_extension_new(UtObject *client, uint8_t major_opcode,
                                       uint8_t first_event,
                                       uint8_t first_error) {
  UtObject *object =
      ut_object_new(sizeof(UtX11MitShmExtension), &object_interface);
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  self->client = client;
  self->major_opcode = major_opcode;
  self->first_event = first_event;
  self->first_error = first_error;
  return object;
}

void ut_x11_mit_shm_extension_enable(UtObject *object,
                                     UtX11MitShmEnableCallback callback,
                                     void *user_data, UtObject *cancel) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_client_send_request_with_reply(
      (UtObject *)self->client, self->major_opcode, 0, request,
      decode_shm_query_version_reply, handle_shm_query_version_error,
      callback_data_new(self, callback, user_data), cancel);
}

uint16_t ut_x11_mit_shm_extension_get_uid(UtObject *object) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  return self->uid;
}

uint16_t ut_x11_mit_shm_extension_get_gid(UtObject *object) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  return self->gid;
}

uint8_t ut_x11_mit_shm_extension_get_pixmap_format(UtObject *object) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  return self->pixmap_format;
}

bool ut_x11_mit_shm_extension_get_shared_pixmaps(UtObject *object) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  return self->shared_pixmaps;
}

uint32_t ut_x11_mit_shm_extension_attach(UtObject *object, uint32_t shmid,
                                         bool read_only) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  uint32_t id = ut_x11_client_create_resource_id(self->client);

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_buffer_append_card32(request, id);
  ut_x11_buffer_append_card32(request, shmid);
  ut_x11_buffer_append_bool(request, read_only);
  ut_x11_buffer_append_padding(request, 3);

  ut_x11_client_send_request((UtObject *)self->client, self->major_opcode, 1,
                             request);

  return id;
}

void ut_x11_mit_shm_extension_detach(UtObject *object, uint32_t segment) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_buffer_append_card32(request, segment);

  ut_x11_client_send_request((UtObject *)self->client, self->major_opcode, 2,
                             request);
}

uint32_t ut_x11_mit_shm_extension_create_pixmap(UtObject *object,
                                                uint32_t drawable,
                                                uint16_t width, uint16_t height,
                                                uint8_t depth, uint32_t segment,
                                                uint32_t offset) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  uint32_t id = ut_x11_client_create_resource_id(self->client);

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_buffer_append_card32(request, id);
  ut_x11_buffer_append_card32(request, drawable);
  ut_x11_buffer_append_card16(request, width);
  ut_x11_buffer_append_card16(request, height);
  ut_x11_buffer_append_card8(request, depth);
  ut_x11_buffer_append_padding(request, 3);
  ut_x11_buffer_append_card32(request, segment);
  ut_x11_buffer_append_card32(request, offset);

  ut_x11_client_send_request((UtObject *)self->client, self->major_opcode, 5,
                             request);

  return id;
}

uint32_t ut_x11_mit_shm_extension_attach_fd(UtObject *object, UtObject *fd,
                                            bool read_only) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  uint32_t id = ut_x11_client_create_resource_id(self->client);

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_buffer_append_card32(request, id);
  ut_x11_buffer_append_bool(request, read_only);
  ut_x11_buffer_append_padding(request, 3);

  ut_x11_buffer_append_fd(request, fd);

  ut_x11_client_send_request((UtObject *)self->client, self->major_opcode, 6,
                             request);

  return id;
}

uint32_t ut_x11_mit_shm_extension_create_segment(
    UtObject *object, uint32_t size, bool read_only,
    UtX11MitShmCreateSegmentCallback callback, void *user_data,
    UtObject *cancel) {
  assert(ut_object_is_x11_mit_shm_extension(object));
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;

  uint32_t id = ut_x11_client_create_resource_id(self->client);

  UtObjectRef request = ut_x11_buffer_new();
  ut_x11_buffer_append_card32(request, id);
  ut_x11_buffer_append_card32(request, size);
  ut_x11_buffer_append_bool(request, read_only);
  ut_x11_buffer_append_padding(request, 3);

  ut_x11_client_send_request_with_reply(
      (UtObject *)self->client, self->major_opcode, 7, request,
      decode_shm_create_segment_reply, handle_shm_create_segment_error,
      callback_data_new(self, callback, user_data), cancel);

  return id;
}

bool ut_object_is_x11_mit_shm_extension(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
