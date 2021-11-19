#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtJpegDecodeCallback)(void *user_data, UtObject *image);

UtObject *ut_jpeg_decoder_new(UtObject *input_stream);

void ut_jpeg_decoder_decode(UtObject *object, UtJpegDecodeCallback callback,
                            void *user_data, UtObject *cancel);

UtObject *ut_jpeg_decoder_decode_sync(UtObject *object);

bool ut_object_is_jpeg_decoder(UtObject *object);
