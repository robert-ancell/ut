#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ut-cancel.h"
#include "ut-end-of-stream.h"
#include "ut-error.h"
#include "ut-general-error.h"
#include "ut-http-header.h"
#include "ut-http-response.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-list.h"
#include "ut-object-private.h"
#include "ut-tcp-client.h"

typedef struct {
  UtObject object;
  unsigned int status_code;
  char *reason_phrase;
  UtObject *headers;
  UtObject *tcp_client;
  bool read_all;
  size_t n_read;
  UtObject *read_cancel;
  UtInputStreamCallback callback;
  void *user_data;
  UtObject *cancel;
} UtHttpResponse;

static const char *get_header(UtHttpResponse *self, const char *name) {
  assert(name != NULL);
  for (size_t i = 0; i < ut_list_get_length(self->headers); i++) {
    UtObject *header = ut_object_list_get_element(self->headers, i);
    if (strcasecmp(ut_http_header_get_name(header), name) == 0) {
      return ut_http_header_get_value(header);
    }
  }

  return NULL;
}

static ssize_t get_content_length(UtHttpResponse *self) {
  const char *value = get_header(self, "Content-Length");
  if (value == NULL) {
    return -1;
  }

  return atoi(value);
}

static size_t read_cb(void *user_data, UtObject *data) {
  UtHttpResponse *self = user_data;
  size_t n_used = 0;
  if (!ut_cancel_is_active(self->cancel)) {
    if (ut_object_implements_error(data)) {
      self->callback(self->user_data, data);
    } else if (ut_object_is_end_of_stream(data)) {
      ssize_t content_length = get_content_length(self);
      if (content_length >= 0) {
        UtObjectRef error =
            ut_general_error_new("Connection closed before end of content");
        self->callback(self->user_data, error);
      } else {
        self->callback(self->user_data, data);
      }
    } else {
      size_t data_length = ut_list_get_length(data);
      ssize_t content_length = get_content_length(self);
      if (content_length >= 0 && self->n_read + data_length > content_length) {
        // Read no more data.
        ut_cancel_activate(self->read_cancel);

        UtObjectRef remaining = ut_list_copy(data);
        ut_list_resize(remaining, content_length - self->n_read);
        n_used = self->callback(self->user_data, remaining);
        if (!self->read_all) {
          ut_list_remove(remaining, 0, n_used);
          UtObjectRef eos = ut_end_of_stream_new(
              ut_list_get_length(remaining) > 0 ? remaining : NULL);
          if (!ut_cancel_is_active(self->cancel)) {
            self->callback(self->user_data, eos);
          }
        }
      } else {
        if (!self->read_all) {
          n_used = self->callback(self->user_data, data);
        }
      }
      self->n_read += n_used;
    }
  }

  // Stop listening for input when done.
  if (ut_cancel_is_active(self->cancel)) {
    ut_cancel_activate(self->read_cancel);
  }

  return n_used;
}

static void start_read(UtHttpResponse *self, bool read_all,
                       UtInputStreamCallback callback, void *user_data,
                       UtObject *cancel) {
  // Clean up after the previous read.
  if (ut_cancel_is_active(self->cancel)) {
    self->read_all = false;
    ut_object_unref(self->read_cancel);
    self->callback = NULL;
    self->user_data = NULL;
    ut_object_unref(self->cancel);
    self->cancel = NULL;
  }

  assert(self->callback == NULL);

  self->read_all = read_all;
  self->read_cancel = ut_cancel_new();
  self->callback = callback;
  self->user_data = user_data;
  self->cancel = cancel != NULL ? ut_object_ref(cancel) : NULL;

  ut_input_stream_read(self->tcp_client, read_cb, self, self->read_cancel);
}

static void ut_http_response_init(UtObject *object) {
  UtHttpResponse *self = (UtHttpResponse *)object;
  self->status_code = 0;
  self->reason_phrase = NULL;
  self->headers = NULL;
  self->tcp_client = NULL;
  self->read_all = false;
  self->n_read = 0;
  self->callback = NULL;
  self->user_data = NULL;
  self->cancel = NULL;
}

static void ut_http_response_cleanup(UtObject *object) {
  UtHttpResponse *self = (UtHttpResponse *)object;
  free(self->reason_phrase);
  ut_object_unref(self->headers);
  ut_object_unref(self->tcp_client);
  if (self->cancel != NULL) {
    ut_object_unref(self->cancel);
  }
}

static void ut_http_response_read(UtObject *object,
                                  UtInputStreamCallback callback,
                                  void *user_data, UtObject *cancel) {
  UtHttpResponse *self = (UtHttpResponse *)object;
  assert(callback != NULL);

  start_read(self, false, callback, user_data, cancel);
}

static void ut_http_response_read_all(UtObject *object,
                                      UtInputStreamCallback callback,
                                      void *user_data, UtObject *cancel) {
  UtHttpResponse *self = (UtHttpResponse *)object;
  assert(callback != NULL);
  assert(self->callback == NULL);

  start_read(self, true, callback, user_data, cancel);
}

static UtInputStreamInterface input_stream_interface = {
    .read = ut_http_response_read, .read_all = ut_http_response_read_all};

static UtObjectInterface object_interface = {
    .type_name = "HttpResponse",
    .init = ut_http_response_init,
    .cleanup = ut_http_response_cleanup,
    .interfaces = {{&ut_input_stream_id, &input_stream_interface},
                   {NULL, NULL}}};

UtObject *ut_http_response_new(unsigned int status_code,
                               const char *reason_phrase, UtObject *headers,
                               UtObject *tcp_client) {
  UtObject *object = ut_object_new(sizeof(UtHttpResponse), &object_interface);
  UtHttpResponse *self = (UtHttpResponse *)object;
  self->status_code = status_code;
  self->reason_phrase = strdup(reason_phrase);
  self->headers = ut_object_ref(headers);
  self->tcp_client = ut_object_ref(tcp_client);
  return object;
}

unsigned int ut_http_response_get_status_code(UtObject *object) {
  assert(ut_object_is_http_response(object));
  UtHttpResponse *self = (UtHttpResponse *)object;
  return self->status_code;
}

const char *ut_http_response_get_reason_phrase(UtObject *object) {
  assert(ut_object_is_http_response(object));
  UtHttpResponse *self = (UtHttpResponse *)object;
  return self->reason_phrase;
}

UtObject *ut_http_response_get_headers(UtObject *object) {
  assert(ut_object_is_http_response(object));
  UtHttpResponse *self = (UtHttpResponse *)object;
  return self->headers;
}

const char *ut_http_response_get_header(UtObject *object, const char *name) {
  assert(ut_object_is_http_response(object));
  UtHttpResponse *self = (UtHttpResponse *)object;
  return get_header(self, name);
}

ssize_t ut_http_response_get_content_length(UtObject *object) {
  assert(ut_object_is_http_response(object));
  UtHttpResponse *self = (UtHttpResponse *)object;
  return get_content_length(self);
}

bool ut_object_is_http_response(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
