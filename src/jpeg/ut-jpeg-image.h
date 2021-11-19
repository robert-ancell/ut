#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef enum {
  UT_JPEG_COLOUR_TYPE_GREYSCALE,
  UT_JPEG_COLOUR_TYPE_TRUECOLOUR,
  UT_JPEG_COLOUR_TYPE_INDEXED_COLOUR,
  UT_JPEG_COLOUR_TYPE_GREYSCALE_WITH_ALPHA,
  UT_JPEG_COLOUR_TYPE_TRUECOLOUR_WITH_ALPHA
} UtJpegColourType;

typedef enum {
  UT_JPEG_INTERLACE_METHOD_NONE,
  UT_JPEG_INTERLACE_METHOD_ADAM7
} UtJpegInterlaceMethod;

UtObject *ut_jpeg_image_new(uint16_t width, uint16_t height, UtObject *data);

void ut_jpeg_image_set_thumbnail(UtObject *object, uint8_t width,
                                 uint8_t height, UtObject *data);

uint8_t ut_jpeg_image_get_thumbnail_width(UtObject *object);

uint8_t ut_jpeg_image_get_thumbnail_height(UtObject *object);

UtObject *ut_jpeg_image_get_thumbnail_data(UtObject *object);

uint16_t ut_jpeg_image_get_width(UtObject *object);

uint16_t ut_jpeg_image_get_height(UtObject *object);

UtObject *ut_jpeg_image_get_data(UtObject *object);

bool ut_object_is_jpeg_image(UtObject *object);
