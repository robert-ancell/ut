#include <assert.h>

#include "ut-raster-image.h"
#include "ut-rasterizer.h"
#include "ut-uint8-array.h"

typedef struct {
  UtObject object;
  UtObject *image;
  double r;
  double g;
  double b;
  double a;
} UtRasterizer;

/*static bool contains_line(size_t x, size_t y, double x0, double y0, double x1,
double y1)
{
   double top = x;
   double bottom = x + 1;
   double left = y;
   double right = y + 1;

   double dx = x1 - x0;
   double dy = y1 - y0;
}*/

static void ut_rasterizer_init(UtObject *object) {
  UtRasterizer *self = (UtRasterizer *)object;
  self->image = NULL;
  self->r = 0.0;
  self->g = 0.0;
  self->b = 0.0;
  self->a = 1.0;
}

static void ut_rasterizer_cleanup(UtObject *object) {
  UtRasterizer *self = (UtRasterizer *)object;
  ut_object_unref(self->image);
}

static UtObjectInterface object_interface = {.type_name = "UtRasterizer",
                                             .init = ut_rasterizer_init,
                                             .cleanup = ut_rasterizer_cleanup};

UtObject *ut_rasterizer_new(UtObject *image) {
  assert(ut_object_is_raster_image(image));
  UtObject *object = ut_object_new(sizeof(UtRasterizer), &object_interface);
  UtRasterizer *self = (UtRasterizer *)object;
  self->image = ut_object_ref(image);
  return object;
}

UtObject *ut_rasterizer_get_image(UtObject *object) {
  assert(ut_object_is_rasterizer(object));
  UtRasterizer *self = (UtRasterizer *)object;
  return self->image;
}

void ut_rasterizer_set_color(UtObject *object, double r, double g, double b,
                             double a) {
  assert(ut_object_is_rasterizer(object));
  UtRasterizer *self = (UtRasterizer *)object;

  self->r = r;
  self->g = g;
  self->b = b;
  self->a = a;
}

void ut_rasterizer_clear(UtObject *object) {
  assert(ut_object_is_rasterizer(object));
  UtRasterizer *self = (UtRasterizer *)object;

  uint8_t r = self->r * 255;
  uint8_t g = self->g * 255;
  uint8_t b = self->b * 255;
  uint8_t a = self->a * 255;

  assert(ut_raster_image_get_format(self->image) ==
         UT_RASTER_IMAGE_FORMAT_RGBA32);
  size_t width = ut_raster_image_get_width(self->image);
  size_t height = ut_raster_image_get_height(self->image);
  uint8_t *data =
      ut_uint8_array_get_data(ut_raster_image_get_data(self->image));

  uint8_t *d = data;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      d[0] = r;
      d[1] = g;
      d[2] = b;
      d[3] = a;
      d += 4;
    }
  }
}

void ut_rasterizer_render_circle(UtObject *object, double cx, double cy,
                                 double radius) {
  assert(ut_object_is_rasterizer(object));
  UtRasterizer *self = (UtRasterizer *)object;

  uint8_t r = self->r * 255;
  uint8_t g = self->g * 255;
  uint8_t b = self->b * 255;
  uint8_t a = self->a * 255;

  assert(ut_raster_image_get_format(self->image) ==
         UT_RASTER_IMAGE_FORMAT_RGBA32);
  size_t width = ut_raster_image_get_width(self->image);
  size_t height = ut_raster_image_get_height(self->image);
  uint8_t *data =
      ut_uint8_array_get_data(ut_raster_image_get_data(self->image));

  double r2 = radius * radius;
  uint8_t *d = data;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      double px = (x + 0.5) - cx;
      double py = (y + 0.5) - cy;
      double pr2 = px * px + py * py;
      if (pr2 <= r2) {
        d[0] = r;
        d[1] = g;
        d[2] = b;
        d[3] = a;
      }
      d += 4;
    }
  }
}

void ut_rasterizer_render_line(UtObject *object, double x1, double y1,
                               double x2, double y2) {}

bool ut_object_is_rasterizer(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
