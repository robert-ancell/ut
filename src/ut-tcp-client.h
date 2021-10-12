#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef void (*UtTcpClientConnectCallback)(void *user_data);

UtObject *ut_tcp_client_new(const char *address, uint16_t port);

void ut_tcp_client_connect(UtObject *object,
                           UtTcpClientConnectCallback callback, void *user_data,
                           UtObject *cancel);

void ut_tcp_client_disconnect(UtObject *object);

bool ut_object_is_tcp_client(UtObject *object);
