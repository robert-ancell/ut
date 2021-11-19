#include <assert.h>
#include <string.h>

#include "ut-error.h"
#include "ut-jpeg-error.h"

typedef struct {
  UtObject object;
} UtJpegError;

static char *ut_jpeg_error_get_description(UtObject *object) {
  return strdup("JPEG Error");
}

static UtErrorInterface error_interface = {.get_description =
                                               ut_jpeg_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtJpegError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_jpeg_error_new() {
  return ut_object_new(sizeof(UtJpegError), &object_interface);
}

bool ut_object_is_jpeg_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
