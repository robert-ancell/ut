#include <assert.h>
#include <stdint.h>

#include "ut-cstring.h"
#include "ut-true-type-font.h"

typedef struct {
  UtObject object;
} UtTrueTypeFont;

static void ut_true_type_font_init(UtObject *object) {
  // UtTrueTypeFont *self = (UtTrueTypeFont *)object;
}

static void ut_true_type_font_cleanup(UtObject *object) {
  // UtTrueTypeFont *self = (UtTrueTypeFont *)object;
}

static UtObjectInterface object_interface = {.type_name = "UtTrueTypeFont",
                                             .init = ut_true_type_font_init,
                                             .cleanup =
                                                 ut_true_type_font_cleanup,
                                             .interfaces = {{NULL, NULL}}};

UtObject *ut_true_type_font_new() {
  UtObject *object = ut_object_new(sizeof(UtTrueTypeFont), &object_interface);
  // UtTrueTypeFont *self = (UtTrueTypeFont *)object;
  return object;
}

bool ut_object_is_true_type_font(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
