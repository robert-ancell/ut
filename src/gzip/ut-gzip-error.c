#include <assert.h>
#include <string.h>

#include "ut-error.h"
#include "ut-gzip-error.h"

typedef struct {
  UtObject object;
} UtGzipError;

static char *ut_gzip_error_get_description(UtObject *object) {
  return strdup("GZip Error");
}

static UtErrorInterface error_interface = {.get_description =
                                               ut_gzip_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtGzipError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_gzip_error_new() {
  return ut_object_new(sizeof(UtGzipError), &object_interface);
}

bool ut_object_is_gzip_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
