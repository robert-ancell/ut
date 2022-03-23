#include "ut.h"

#include <stdio.h>

int main(int argc, char **argv) {
  UtObjectRef data = ut_uint8_array_new();
  ut_list_resize(data, 400);
  printf("%s\n", ut_object_to_string(data));
  UtObjectRef image =
      ut_raster_image_new(10, 10, UT_RASTER_IMAGE_FORMAT_RGBA32, data);
  UtObjectRef r = ut_rasterizer_new(image);
  ut_rasterizer_set_color(r, 1.0, 0.0, 0.0, 1.0);
  ut_rasterizer_clear(r);
  printf("%s\n", ut_object_to_string(data));
}
