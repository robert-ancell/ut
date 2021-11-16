#include <assert.h>
#include <string.h>

#include "ut-error.h"
#include "ut-png-error.h"

typedef struct {
  UtObject object;
} UtPngError;

static char *ut_png_error_get_description(UtObject *object) {
  return strdup("PNG Error");
}

static UtErrorInterface error_interface = {.get_description =
                                               ut_png_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtPngError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_png_error_new() {
  return ut_object_new(sizeof(UtPngError), &object_interface);
}

bool ut_object_is_png_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
