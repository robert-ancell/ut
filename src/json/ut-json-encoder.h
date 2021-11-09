#include <stdbool.h>

#include "ut-object.h"

#pragma once

UtObject *ut_json_encoder_new();

char *ut_json_encoder_encode(UtObject *object, UtObject *message);

bool ut_object_is_json_encoder(UtObject *object);
