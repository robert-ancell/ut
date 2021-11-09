#include "ut-object.h"

#pragma once

char *ut_json_encode(UtObject *object);

UtObject *ut_json_decode(const char *text);
