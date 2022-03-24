#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_rasterizer_new(UtObject *image);

UtObject *ut_rasterizer_get_image(UtObject *object);

void ut_rasterizer_set_color(UtObject *object, double r, double g, double b,
                             double a);

void ut_rasterizer_clear(UtObject *object);

void ut_rasterizer_render_circle(UtObject *object, double cx, double cy,
                                 double radius);

void ut_rasterizer_render_line(UtObject *object, double x1, double y1,
                               double x2, double y2);

bool ut_object_is_rasterizer(UtObject *object);
