#include <assert.h>
#include <string.h>

#include "ut-error.h"
#include "ut-zlib-error.h"

typedef struct {
  UtObject object;
} UtZlibError;

static char *ut_zlib_error_get_description(UtObject *object) {
  return strdup("zlib Error");
}

static UtErrorInterface error_interface = {.get_description =
                                               ut_zlib_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtZlibError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_zlib_error_new() {
  return ut_object_new(sizeof(UtZlibError), &object_interface);
}

bool ut_object_is_zlib_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
