#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_gzip_decoder_new(UtObject *input_stream);

bool ut_object_is_gzip_decoder(UtObject *object);
