#include <assert.h>
#include <stdint.h>

#include "ut-cstring.h"
#include "ut-jpeg-image.h"
#include "ut-list.h"

typedef struct {
  UtObject object;
  uint8_t thumbnail_width;
  uint8_t thumbnail_height;
  UtObject *thumbnail_data;
  uint16_t width;
  uint16_t height;
  UtObject *data;
} UtJpegImage;

static void ut_jpeg_image_init(UtObject *object) {
  UtJpegImage *self = (UtJpegImage *)object;
  self->thumbnail_width = 0;
  self->thumbnail_height = 0;
  self->thumbnail_data = NULL;
  self->width = 0;
  self->height = 0;
  self->data = NULL;
}

static void ut_jpeg_image_cleanup(UtObject *object) {
  UtJpegImage *self = (UtJpegImage *)object;
  ut_object_unref(self->thumbnail_data);
  ut_object_unref(self->data);
}

static char *ut_jpeg_image_to_string(UtObject *object) {
  UtJpegImage *self = (UtJpegImage *)object;
  return ut_cstring_new_printf("<UtJpegImage>(width: %d, height: %d)",
                               self->width, self->height);
}

static UtObjectInterface object_interface = {.type_name = "UtJpegImage",
                                             .init = ut_jpeg_image_init,
                                             .cleanup = ut_jpeg_image_cleanup,
                                             .to_string =
                                                 ut_jpeg_image_to_string,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_jpeg_image_new(uint16_t width, uint16_t height, UtObject *data) {
  UtObject *object = ut_object_new(sizeof(UtJpegImage), &object_interface);
  UtJpegImage *self = (UtJpegImage *)object;

  // assert(ut_list_get_length(data) == width * height * 3);

  self->width = width;
  self->height = height;
  self->data = ut_object_ref(data);
  return object;
}

void ut_jpeg_image_set_thumbnail(UtObject *object, uint8_t width,
                                 uint8_t height, UtObject *data) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;

  assert((data != NULL ? ut_list_get_length(data) : 0) == width * height * 3);

  self->thumbnail_width = width;
  self->thumbnail_height = height;
  ut_object_unref(self->thumbnail_data);
  self->thumbnail_data = ut_object_ref(data);
}

uint8_t ut_jpeg_image_get_thumbnail_width(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->thumbnail_width;
}

uint8_t ut_jpeg_image_get_thumbnail_height(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->thumbnail_height;
}

UtObject *ut_jpeg_image_get_thumbnail_data(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->thumbnail_data;
}

uint16_t ut_jpeg_image_get_width(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->width;
}

uint16_t ut_jpeg_image_get_height(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->height;
}

UtObject *ut_jpeg_image_get_data(UtObject *object) {
  assert(ut_object_is_jpeg_image(object));
  UtJpegImage *self = (UtJpegImage *)object;
  return self->data;
}

bool ut_object_is_jpeg_image(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
