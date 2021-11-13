#include <assert.h>
#include <string.h>

#include "ut-deflate-error.h"
#include "ut-error.h"

typedef struct {
  UtObject object;
} UtDeflateError;

static char *ut_deflate_error_get_description(UtObject *object) {
  return strdup("Deflate Error");
}

static UtErrorInterface error_interface = {
    .get_description = ut_deflate_error_get_description};

static UtObjectInterface object_interface = {
    .type_name = "UtDeflateError",
    .interfaces = {{&ut_error_id, &error_interface}, {NULL, NULL}}};

UtObject *ut_deflate_error_new() {
  return ut_object_new(sizeof(UtDeflateError), &object_interface);
}

bool ut_object_is_deflate_error(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
