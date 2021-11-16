#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef enum {
  UT_PNG_COLOUR_TYPE_GREYSCALE,
  UT_PNG_COLOUR_TYPE_TRUECOLOUR,
  UT_PNG_COLOUR_TYPE_INDEXED_COLOUR,
  UT_PNG_COLOUR_TYPE_GREYSCALE_WITH_ALPHA,
  UT_PNG_COLOUR_TYPE_TRUECOLOUR_WITH_ALPHA
} UtPngColourType;

typedef enum {
  UT_PNG_INTERLACE_METHOD_NONE,
  UT_PNG_INTERLACE_METHOD_ADAM7
} UtPngInterlaceMethod;

UtObject *ut_png_image_new(uint32_t width, uint32_t height, uint8_t bit_depth,
                           UtPngColourType colour_type, UtObject *data);

void ut_png_image_set_interlace_method(UtObject *object,
                                       UtPngInterlaceMethod method);

UtPngInterlaceMethod ut_png_image_get_interlace_method(UtObject *object);

uint32_t ut_png_image_get_width(UtObject *object);

uint32_t ut_png_image_get_height(UtObject *object);

uint8_t ut_png_image_get_bit_depth(UtObject *object);

UtPngColourType ut_png_image_get_colour_type(UtObject *object);

UtObject *ut_png_image_get_data(UtObject *object);

bool ut_object_is_png_image(UtObject *object);
