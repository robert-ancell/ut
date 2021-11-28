#include <assert.h>

#include "ut-x11-extension.h"
#include "ut-x11-mit-shm-extension.h"

typedef struct {
  UtObject object;
  UtObject *client;
  uint8_t major_opcode;
  uint8_t first_event;
  uint8_t first_error;
} UtX11MitShmExtension;

static void ut_x11_mit_shm_extension_init(UtObject *object) {
  UtX11MitShmExtension *self = (UtX11MitShmExtension *)object;
  self->major_opcode = 0;
  self->first_event = 0;
  self->first_error = 0;
}

static UtObject *ut_x11_mit_shm_extension_decode_event(UtObject *object,
                                                       UtObject *data) {
  return NULL;
}

static UtObject *ut_x11_mit_shm_extension_decode_error(UtObject *object,
                                                       UtObject *data) {
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

bool ut_object_is_x11_mit_shm_extension(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
