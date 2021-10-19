#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ut-object.h"

#pragma once

typedef void (*UtTcpClientConnectCallback)(void *user_data);

typedef size_t (*UtTcpClientReadCallback)(void *user_data, UtObject *data);

UtObject *ut_tcp_client_new(const char *address, uint16_t port);

void ut_tcp_client_connect(UtObject *object,
                           UtTcpClientConnectCallback callback, void *user_data,
                           UtObject *cancel);

void ut_tcp_client_read(UtObject *object, size_t count,
                        UtTcpClientReadCallback callback, void *user_data,
                        UtObject *cancel);

void ut_tcp_client_read_stream(UtObject *object, size_t block_size,
                               UtTcpClientReadCallback callback,
                               void *user_data, UtObject *cancel);

void ut_tcp_client_read_all(UtObject *object, UtTcpClientReadCallback callback,
                            void *user_data, UtObject *cancel);

void ut_tcp_client_disconnect(UtObject *object);

bool ut_object_is_tcp_client(UtObject *object);
