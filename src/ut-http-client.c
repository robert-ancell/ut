#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "ut-cancel.h"
#include "ut-cstring.h"
#include "ut-event-loop.h"
#include "ut-http-client.h"
#include "ut-http-header.h"
#include "ut-http-response.h"
#include "ut-input-stream.h"
#include "ut-list.h"
#include "ut-object-array.h"
#include "ut-output-stream.h"
#include "ut-string.h"
#include "ut-tcp-client.h"
#include "ut-uint8-array.h"
#include "ut-uint8-list.h"

typedef struct {
  UtObject object;
} UtHttpClient;

typedef struct {
  UtHttpClient *client;
  UtObject *tcp_client;
  char *host;
  char *method;
  char *path;
  UtHttpResponseCallback callback;
  void *callback_user_data;
  unsigned int response_status_code;
  char *reason_phrase;
  UtObject *response_headers;
  UtObject *header_read_cancel;
} HttpRequest;

HttpRequest *http_request_new(const char *host, uint16_t port,
                              const char *method, const char *path,
                              UtHttpResponseCallback callback,
                              void *callback_user_data) {
  HttpRequest *request = malloc(sizeof(HttpRequest));
  request->tcp_client = ut_tcp_client_new(host, port);
  request->host = strdup(host);
  request->method = strdup(method);
  request->path = strdup(path);
  request->callback = callback;
  request->callback_user_data = callback_user_data;
  request->response_status_code = 0;
  request->reason_phrase = NULL;
  request->response_headers = ut_object_array_new();
  request->header_read_cancel = ut_cancel_new();
  return request;
}

void http_request_free(HttpRequest *request) {
  ut_object_unref(request->tcp_client);
  free(request->host);
  free(request->method);
  free(request->path);
  free(request->reason_phrase);
  ut_object_unref(request->response_headers);
  ut_object_unref(request->header_read_cancel);
  free(request);
}

static bool parse_uri(const char *uri, char **scheme, char **user_info,
                      char **host, uint16_t *port, char **path, char **query,
                      char **fragment) {
  const char *scheme_start = uri, *scheme_end = uri;
  while (*scheme_end != '\0' && *scheme_end != ':') {
    scheme_end++;
  }
  if (*scheme_end != ':') {
    return false;
  }
  if (scheme != NULL) {
    *scheme = strndup(scheme_start, scheme_end - scheme_start);
  }

  const char *hier_part_start = scheme_end + 1, *hier_part_end;
  if (strncmp(hier_part_start, "//", 2) == 0) {
    const char *authority_start = hier_part_start + 2,
               *authority_end = authority_start;
    while (*authority_end != '\0' && *authority_end != '/' &&
           *authority_end != '?' && *authority_end != '#') {
      authority_end++;
    }

    const char *userinfo_start = authority_start,
               *userinfo_end = userinfo_start;
    while (userinfo_end != authority_end && *userinfo_end != '@') {
      userinfo_end++;
    }
    const char *host_start = userinfo_end;
    if (userinfo_end != authority_end) {
      if (user_info != NULL) {
        *user_info = strndup(userinfo_start, userinfo_end - userinfo_start);
      }
      host_start = userinfo_end + 1;
    } else {
      if (user_info != NULL) {
        *user_info = NULL;
      }
      host_start = authority_start;
    }

    const char *host_end = host_start;
    while (host_end != authority_end && *host_end != ':') {
      host_end++;
    }
    if (host != NULL) {
      *host = strndup(userinfo_start, userinfo_end - userinfo_start);
    }

    if (*host_end == ':') {
      const char *port_start = host_end + 1, *port_end = authority_end;
      if (port != NULL) {
        ut_cstring_ref port_string = strndup(port_start, port_end - port_start);
        *port = atoi(port_string);
      }
    } else {
      if (port != NULL) {
        *port = 0;
      }
    }
    hier_part_end = authority_end;
  } else {
    if (user_info != NULL) {
      *user_info = NULL;
    }
    if (host != NULL) {
      *host = NULL;
    }
    if (port != NULL) {
      *port = 0;
    }
    hier_part_end = hier_part_start;
  }

  const char *path_start = hier_part_end, *path_end = path_start;
  while (*path_end != '\0' && *path_end != '?' && *path_end != '#') {
    path_end++;
  }
  if (path != NULL) {
    *path = strndup(path_start, path_end - path_start);
  }

  if (*path_end == '?') {
    const char *query_start = path_end + 1, *query_end;
    while (*query_end != '\0' && *query_end != '#') {
      query_end++;
    }
    if (query != NULL) {
      *query = strndup(query_start, query_end - query_start);
    }
    path_start = query_end;
  } else {
    if (query != NULL) {
      *query = NULL;
    }
  }

  if (*path_end == '#') {
    const char *fragment_start = path_end + 1, *fragment_end;
    while (*fragment_end != '\0' && *fragment_end != '#') {
      fragment_end++;
    }
    if (fragment != NULL) {
      *fragment = strndup(fragment_start, fragment_end - fragment_start);
    }
    path_start = fragment_end;
  } else {
    if (fragment != NULL) {
      *fragment = NULL;
    }
  }

  return true;
}

static ssize_t find_line_end(const uint8_t *data, size_t data_length,
                             size_t offset) {
  for (size_t i = offset; i < data_length - 1; i++) {
    if (data[i] == '\r' && data[i + 1] == '\n') {
      return i;
    }
  }

  return -1;
}

static ssize_t find_character(const uint8_t *data, size_t data_length,
                              size_t offset, char character) {
  for (size_t i = offset; i < data_length; i++) {
    if (data[i] == character) {
      return i;
    }
  }

  return -1;
}

static char *get_string(const uint8_t *data, size_t start, size_t end) {
  while (start < end && data[start] == ' ') {
    start++;
  }
  while (end > start && data[end - 1] == ' ') {
    end--;
  }
  return strndup((const char *)data + start, end - start);
}

static bool parse_status_line(HttpRequest *request, const uint8_t *data,
                              size_t data_length) {
  size_t protocol_version_start = 0;
  ssize_t protocol_version_end =
      find_character(data, data_length, protocol_version_start, ' ');
  if (protocol_version_end < 0) {
    return false;
  }

  size_t response_status_code_start = protocol_version_end + 1;
  ssize_t response_status_code_end =
      find_character(data, data_length, response_status_code_start, ' ');
  if (response_status_code_end < 0) {
    return false;
  }

  size_t reason_phrase_start = response_status_code_end + 1;
  size_t reason_phrase_end = data_length;

  ut_cstring_ref response_status_code =
      get_string(data, response_status_code_start, response_status_code_end);
  request->response_status_code = atoi(response_status_code);
  request->reason_phrase =
      get_string(data, reason_phrase_start, reason_phrase_end);

  return true;
}

static bool parse_header(HttpRequest *request, const uint8_t *data,
                         size_t data_length) {
  size_t name_start = 0;
  ssize_t name_end = find_character(data, data_length, name_start, ':');
  if (name_end < 0) {
    return false;
  }

  size_t value_start = name_end + 1;
  size_t value_end = data_length;

  ut_cstring_ref name = get_string(data, name_start, name_end);
  ut_cstring_ref value = get_string(data, value_start, value_end);
  UtObjectRef header = ut_http_header_new(name, value);

  ut_list_append(request->response_headers, header);

  return true;
}

static void response_cb(void *user_data) {
  HttpRequest *request = user_data;

  UtObjectRef response = ut_http_response_new(
      request->response_status_code, request->reason_phrase,
      request->response_headers, request->tcp_client);
  if (request->callback != NULL) {
    request->callback(request->callback_user_data, response);
  }
}

static size_t read_cb(void *user_data, UtObject *data) {
  HttpRequest *request = user_data;

  const uint8_t *buffer = ut_uint8_array_get_data(data);
  size_t buffer_length = ut_list_get_length(data);
  size_t offset = 0;
  while (true) {
    size_t line_start = offset;
    ssize_t line_end = find_line_end(buffer, buffer_length, line_start);
    if (line_end < 0) {
      break;
    }
    offset = line_end + 2;

    // Ends on empty line.
    if ((size_t)line_end == line_start) {
      ut_event_loop_add_delay(0, response_cb, request, NULL); // FIXME: cancel
      ut_cancel_activate(request->header_read_cancel);
      break;
    }

    if (request->reason_phrase == NULL) {
      assert(parse_status_line(request, buffer + line_start,
                               line_end - line_start));
    } else {
      assert(parse_header(request, buffer + line_start, line_end - line_start));
    }
  }

  return offset;
}

static void connect_cb(void *user_data) {
  HttpRequest *request = user_data;

  UtObjectRef header = ut_string_new(request->method);
  ut_string_append(header, " ");
  ut_string_append(header, request->path);
  ut_string_append(header, " HTTP/1.1\r\n");
  ut_string_append(header, "Host: ");
  ut_string_append(header, request->host);
  ut_string_append(header, "\r\n");
  ut_string_append(header, "\r\n");
  UtObjectRef utf8 = ut_string_get_utf8(header);
  ut_output_stream_write(request->tcp_client, utf8);
  ut_input_stream_read(request->tcp_client, read_cb, request,
                       request->header_read_cancel);
}

static UtObjectInterface object_interface = {.type_name = "HttpClient"};

UtObject *ut_http_client_new() {
  return ut_object_new(sizeof(UtHttpClient), &object_interface);
}

void ut_http_client_send_request(UtObject *object, const char *method,
                                 const char *uri,
                                 UtHttpResponseCallback callback,
                                 void *user_data, UtObject *cancel) {
  assert(ut_object_is_http_client(object));
  // UtHttpClient *self = (UtHttpClient *)object;

  ut_cstring_ref scheme = NULL;
  ut_cstring_ref host = NULL;
  ut_cstring_ref path = NULL;
  uint16_t port;
  assert(parse_uri(uri, &scheme, NULL, &host, &port, &path, NULL, NULL));
  assert(strcmp(scheme, "http") == 0);
  assert(host != NULL);
  if (strcmp(path, "") == 0) {
    ut_cstring_set(&path, "/");
  }
  if (port == 0) {
    port = 80;
  }

  HttpRequest *request =
      http_request_new(host, port, method, path, callback, user_data);
  ut_tcp_client_connect(request->tcp_client, connect_cb, request, cancel);
}

bool ut_object_is_http_client(UtObject *object) {
  return ut_object_is_type(object, &object_interface);
}
