#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_utf8_decoder_new(UtObject *input);

bool ut_object_is_utf8_decoder(UtObject *object);
