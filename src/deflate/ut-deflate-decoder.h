#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_deflate_decoder_new(UtObject *input_stream);

bool ut_object_is_deflate_decoder(UtObject *object);
