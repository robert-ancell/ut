#include "ut.h"

#include <stdio.h>

int main(int argc, char **argv) {
  size_t width = 100;
  size_t height = 100;
  UtObjectRef data = ut_uint8_array_new();
  ut_list_resize(data, 4 * width * height);
  UtObjectRef image =
      ut_raster_image_new(width, height, UT_RASTER_IMAGE_FORMAT_RGBA32, data);
  UtObjectRef r = ut_rasterizer_new(image);
  ut_rasterizer_set_color(r, 1.0, 1.0, 1.0, 1.0);
  ut_rasterizer_clear(r);
  ut_rasterizer_set_color(r, 0.0, 0.0, 0.0, 1.0);
  ut_rasterizer_render_circle(r, width * 0.5, height * 0.5, width * 0.4);
  ut_rasterizer_set_color(r, 1.0, 1.0, 1.0, 1.0);
  ut_rasterizer_render_circle(r, width * 0.5, height * 0.5, width * 0.2);

  UtObjectRef ppm = ut_string_new("");
  ut_string_append(ppm, "P3\n");
  ut_string_append_printf(ppm, "%zi %zi\n", width, height);
  ut_string_append(ppm, "255\n");
  uint8_t *d = ut_uint8_array_get_data(data);
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      ut_string_append_printf(ppm, "%d %d %d\n", d[0], d[1], d[2]);
      d += 4;
    }
  }
  printf("%s\n", ut_string_get_text(ppm));
}
