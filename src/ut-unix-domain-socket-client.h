#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

UtObject *ut_unix_domain_socket_client_new(const char *path);

void ut_unix_domain_socket_client_connect(UtObject *object);

void ut_unix_domain_socket_client_disconnect(UtObject *object);

bool ut_object_is_unix_domain_socket_client(UtObject *object);
