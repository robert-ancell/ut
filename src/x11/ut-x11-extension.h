#include <stdbool.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef struct {
  UtObject *(*decode_event)(UtObject *object, UtObject *data);
  UtObject *(*decode_generic_event)(UtObject *object, uint8_t major_opcode,
                                    uint16_t code, UtObject *data);
  UtObject *(*decode_error)(UtObject *object, UtObject *data);
  void (*close)(UtObject *object);
} UtX11ExtensionInterface;

extern int ut_x11_extension_id;

UtObject *ut_x11_extension_decode_event(UtObject *object, UtObject *data);

UtObject *ut_x11_extension_decode_generic_event(UtObject *object,
                                                uint8_t major_opcode,
                                                uint16_t code, UtObject *data);

UtObject *ut_x11_extension_decode_error(UtObject *object, UtObject *data);

void ut_x11_extension_close(UtObject *object);

bool ut_object_implements_x11_extension(UtObject *object);
