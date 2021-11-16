#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_zlib_decoder_new(UtObject *input_stream);

bool ut_object_is_zlib_decoder(UtObject *object);
