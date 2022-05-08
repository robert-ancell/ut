#include <assert.h>
#include <math.h>

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

static void set_pixel(uint8_t *data, uint8_t r, uint8_t g, uint8_t b,
                      uint8_t a) {
  if (a == 255) {
    data[0] = r;
    data[1] = g;
    data[2] = b;
    data[3] = a;
  } else if (a != 0) {
    double a_ = a / 255.0;
    data[0] = data[0] * (1.0 - a_) + r * a_;
    data[1] = data[1] * (1.0 - a_) + g * a_;
    data[2] = data[2] * (1.0 - a_) + b * a_;
    data[3] = data[3] * (1.0 - a_) + a * a_;
  }
}

static double circle_overlap(double px, double py, double pixel_size, double cx,
                             double cy, double radius2) {
  double dx = floor(fabs(px - cx));
  double dy = floor(fabs(py - cy));
  double pr2_min = dx * dx + dy * dy;
  dx += pixel_size;
  dy += pixel_size;
  double pr2_max = dx * dx + dy * dy;

  if (radius2 >= pr2_max) {
    return 1.0;
  } else if (pixel_size <= 0.125) {
    return 0.5;
  } else if (radius2 >= pr2_min) {
    double subpixel_size = pixel_size * 0.5;
    double subpixel_shift = pixel_size * 0.5;
    return circle_overlap(px - subpixel_shift, py - subpixel_shift,
                          subpixel_size, cx, cy, radius2) *
               0.25 +
           circle_overlap(px + subpixel_shift, py - subpixel_shift,
                          subpixel_size, cx, cy, radius2) *
               0.25 +
           circle_overlap(px - subpixel_shift, py + subpixel_shift,
                          subpixel_size, cx, cy, radius2) *
               0.25 +
           circle_overlap(px + subpixel_shift, py + subpixel_shift,
                          subpixel_size, cx, cy, radius2) *
               0.25;
    return 0.5;
  } else {
    return 0.0;
  }
}

static bool edge_function(double x, double y, double x1, double y1, double x2,
                          double y2) {
  return (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1) >= 0;
}

static bool inside_triangle(double x, double y, double x1, double y1, double x2,
                            double y2, double x3, double y3) {
  return edge_function(x, y, x1, y1, x2, y2) &&
         edge_function(x, y, x2, y2, x3, y3) &&
         edge_function(x, y, x3, y3, x1, y1);
}

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
      set_pixel(d, r, g, b, a);
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

  // FIXME left, right, top, bottom
  // FIXME: Skip center

  double r2 = radius * radius;
  uint8_t *d = data;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      double overlap = circle_overlap(x + 0.5, y + 0.5, 1.0, cx, cy, r2);
      if (overlap >= 0.0) {
        set_pixel(d, r, g, b, a * overlap);
      }
      d += 4;
    }
  }
}

void ut_rasterizer_render_triangle(UtObject *object, double x1, double y1,
                                   double x2, double y2, double x3, double y3) {
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

  // FIXME left, right, top, bottom

  uint8_t *d = data;
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      if (inside_triangle(x + 0.5, y + 0.5, x1, y1, x2, y2, x3, y3)) {
        set_pixel(d, r, g, b, a);
      }
      d += 4;
    }
  }
}

void ut_rasterizer_render_rectangle(UtObject *object, double x, double y,
                                    double width, double height) {
  ut_rasterizer_render_triangle(object, x, y, x, y + height, x + width, y);
  ut_rasterizer_render_triangle(object, x, y + height, x + width, y + height,
                                x + width, y);
}

#include <stdio.h>

void ut_rasterizer_render_line(UtObject *object, double x1, double y1,
                               double x2, double y2, double width) {
  double dx = x2 - x1;
  double dy = y2 - y1;

  double length = sqrt(dx * dx + dy * dy);
  double ox = dy * 0.5 * width / length;
  double oy = -dx * 0.5 * width / length;

  ut_rasterizer_render_triangle(object, x1 + ox, y1 + oy, x1 - ox, y1 - oy,
                                x2 + ox, y2 + oy);
  ut_rasterizer_render_triangle(object, x1 - ox, y1 - oy, x2 - ox, y2 - oy,
                                x2 + ox, y2 + oy);
}

bool ut_object_is_rasterizer(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
