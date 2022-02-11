#include <assert.h>

#include "ut-x11-colormap-error.h"
#include "ut-x11-error.h"

typedef struct {
  UtObject object;
  uint32_t colormap;
} UtX11ColormapError;

static void ut_x11_colormap_error_init(UtObject *object) {
  UtX11ColormapError *self = (UtX11ColormapError *)object;
  self->colormap = 0;
}

static UtX11ErrorInterface x11_error_interface = {};

static UtObjectInterface object_interface = {
    .type_name = "UtX11ColormapError",
    .init = ut_x11_colormap_error_init,
    .interfaces = {{&ut_x11_error_id, &x11_error_interface}, {NULL, NULL}}};

UtObject *ut_x11_colormap_error_new(uint32_t colormap) {
  UtObject *object =
      ut_object_new(sizeof(UtX11ColormapError), &object_interface);
  UtX11ColormapError *self = (UtX11ColormapError *)object;
  self->colormap = colormap;
  return object;
}

uint32_t ut_x11_colormap_error_get_colormap(UtObject *object) {
  assert(ut_object_is_x11_colormap_error(object));
  UtX11ColormapError *self = (UtX11ColormapError *)object;
  return self->colormap;
}

bool ut_object_is_x11_colormap_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
