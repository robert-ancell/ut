#include <stdbool.h>

#include "ut-object.h"

#pragma once

typedef void (*UtHttpResponseCallback)(void *user_data, UtObject *response);

UtObject *ut_http_client_new();

void ut_http_client_send_request(UtObject *object, const char *method,
                                 const char *uri,
                                 UtHttpResponseCallback callback,
                                 void *user_data, UtObject *cancel);

bool ut_object_is_http_client(UtObject *object);
