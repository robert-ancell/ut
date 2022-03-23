#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef enum {
  UT_RASTER_IMAGE_FORMAT_A8,
  UT_RASTER_IMAGE_FORMAT_RGB24,
  UT_RASTER_IMAGE_FORMAT_RGBX32,
  UT_RASTER_IMAGE_FORMAT_RGBA32,
  UT_RASTER_IMAGE_FORMAT_ARGB32,
} UtRasterImageFormat;

UtObject *ut_raster_image_new(size_t width, size_t height,
                              UtRasterImageFormat format, UtObject *data);

size_t ut_raster_image_get_width(UtObject *object);

size_t ut_raster_image_get_height(UtObject *object);

UtRasterImageFormat ut_raster_image_get_format(UtObject *object);

UtObject *ut_raster_image_get_data(UtObject *object);

bool ut_object_is_raster_image(UtObject *object);
