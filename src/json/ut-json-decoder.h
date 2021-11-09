#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_json_decoder_new(UtObject *input_stream);

UtObject *ut_json_decode(const char *text);

bool ut_object_is_json_decoder(UtObject *object);
