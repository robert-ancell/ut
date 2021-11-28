#include <assert.h>

#include "ut-x11-big-requests-extension.h"
#include "ut-x11-extension.h"

typedef struct {
  UtObject object;
  UtObject *client;
  uint8_t major_opcode;
} UtX11BigRequestsExtension;

static void ut_x11_big_requests_extension_init(UtObject *object) {
  UtX11BigRequestsExtension *self = (UtX11BigRequestsExtension *)object;
  self->major_opcode = 0;
}

static void ut_x11_big_requests_extension_close(UtObject *object) {
  UtX11BigRequestsExtension *self = (UtX11BigRequestsExtension *)object;
  self->client = NULL;
}

static UtX11ExtensionInterface x11_extension_interface = {
    .close = ut_x11_big_requests_extension_close};

static UtObjectInterface object_interface = {
    .type_name = "UtX11BigRequestsExtension",
    .init = ut_x11_big_requests_extension_init,
    .interfaces = {{&ut_x11_extension_id, &x11_extension_interface},
                   {NULL, NULL}}};

UtObject *ut_x11_big_requests_extension_new(UtObject *client,
                                            uint8_t major_opcode) {
  UtObject *object =
      ut_object_new(sizeof(UtX11BigRequestsExtension), &object_interface);
  UtX11BigRequestsExtension *self = (UtX11BigRequestsExtension *)object;
  self->client = client;
  self->major_opcode = major_opcode;
  return object;
}

bool ut_object_is_x11_big_requests_extension(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
