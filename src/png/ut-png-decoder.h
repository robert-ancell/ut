#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtPngDecodeCallback)(void *user_data, UtObject *image);

UtObject *ut_png_decoder_new(UtObject *input_stream);

void ut_png_decoder_decode(UtObject *object, UtPngDecodeCallback callback,
                           void *user_data, UtObject *cancel);

UtObject *ut_png_decoder_decode_sync(UtObject *object);

bool ut_object_is_png_decoder(UtObject *object);
