#include <stdbool.h>
#include <sys/types.h>

#include "ut-object.h"

#pragma once

UtObject *ut_http_response_new(unsigned int status_code,
                               const char *reason_phrase, UtObject *headers,
                               UtObject *tcp_client);

unsigned int ut_http_response_get_status_code(UtObject *object);

const char *ut_http_response_get_reason_phrase(UtObject *object);

UtObject *ut_http_response_get_headers(UtObject *object);

const char *ut_http_response_get_header(UtObject *object, const char *name);

ssize_t ut_http_response_get_content_length(UtObject *object);

bool ut_object_is_http_response(UtObject *object);
