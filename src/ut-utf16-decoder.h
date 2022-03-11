#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_utf16_decoder_new(UtObject *input);

bool ut_object_is_utf16_decoder(UtObject *object);
