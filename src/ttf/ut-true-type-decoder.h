#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_true_type_decoder_new(UtObject *data);

void ut_true_type_decoder_decode(UtObject *object);

bool ut_object_is_true_type_decoder(UtObject *object);
