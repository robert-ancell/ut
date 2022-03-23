#include <assert.h>

#include "ut-raster-image.h"

typedef struct {
  UtObject object;
  size_t width;
  size_t height;
  UtRasterImageFormat format;
  UtObject *data;
} UtRasterImage;

static void ut_raster_image_init(UtObject *object) {
  UtRasterImage *self = (UtRasterImage *)object;
  self->width = 0;
  self->height = 0;
  self->format = 0;
  self->data = NULL;
}

static void ut_raster_image_cleanup(UtObject *object) {
  UtRasterImage *self = (UtRasterImage *)object;
  ut_object_unref(self->data);
}

static UtObjectInterface object_interface = {.type_name = "UtRasterImage",
                                             .init = ut_raster_image_init,
                                             .cleanup =
                                                 ut_raster_image_cleanup};

UtObject *ut_raster_image_new(size_t width, size_t height,
                              UtRasterImageFormat format, UtObject *data) {
  UtObject *object = ut_object_new(sizeof(UtRasterImage), &object_interface);
  UtRasterImage *self = (UtRasterImage *)object;
  self->width = width;
  self->height = height;
  self->format = format;
  self->data = ut_object_ref(data);
  return object;
}

size_t ut_raster_image_get_width(UtObject *object) {
  assert(ut_object_is_raster_image(object));
  UtRasterImage *self = (UtRasterImage *)object;
  return self->width;
}

size_t ut_raster_image_get_height(UtObject *object) {
  assert(ut_object_is_raster_image(object));
  UtRasterImage *self = (UtRasterImage *)object;
  return self->height;
}

UtRasterImageFormat ut_raster_image_get_format(UtObject *object) {
  assert(ut_object_is_raster_image(object));
  UtRasterImage *self = (UtRasterImage *)object;
  return self->format;
}

UtObject *ut_raster_image_get_data(UtObject *object) {
  assert(ut_object_is_raster_image(object));
  UtRasterImage *self = (UtRasterImage *)object;
  return self->data;
}

bool ut_object_is_raster_image(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
