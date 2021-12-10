ut_sources = [
  'deflate/ut-deflate-decoder.c',
  'deflate/ut-deflate-error.c',
  'dbus/ut-dbus-array.c',
  'dbus/ut-dbus-auth-client.c',
  'dbus/ut-dbus-client.c',
  'dbus/ut-dbus-dict.c',
  'dbus/ut-dbus-error.c',
  'dbus/ut-dbus-message.c',
  'dbus/ut-dbus-message-decoder.c',
  'dbus/ut-dbus-message-encoder.c',
  'dbus/ut-dbus-object-path.c',
  'dbus/ut-dbus-signature.c',
  'dbus/ut-dbus-struct.c',
  'dbus/ut-dbus-unix-fd.c',
  'dbus/ut-dbus-variant.c',
  'gzip/ut-gzip-decoder.c',
  'gzip/ut-gzip-error.c',
  'http/ut-http-client.c',
  'http/ut-http-header.c',
  'http/ut-http-response.c',
  'png/ut-png-decoder.c',
  'png/ut-png-error.c',
  'png/ut-png-image.c',
  'json/ut-json.c',
  'json/ut-json-encoder.c',
  'ut-assert.c',
  'ut-base64.c',
  'ut-boolean.c',
  'ut-cancel.c',
  'ut-constant-utf8-string.c',
  'ut-constant-uint8-array.c',
  'ut-cstring.c',
  'ut-event-loop.c',
  'ut-error.c',
  'ut-fd-input-stream.c',
  'ut-fd-output-stream.c',
  'ut-file.c',
  'ut-float64.c',
  'ut-float64-array.c',
  'ut-float64-list.c',
  'ut-general-error.c',
  'ut-hash-table.c',
  'ut-huffman-decoder.c',
  'ut-input-stream.c',
  'ut-input-stream-multiplexer.c',
  'ut-int16.c',
  'ut-int32.c',
  'ut-int32-array.c',
  'ut-int32-list.c',
  'ut-int32-subarray.c',
  'ut-int64.c',
  'ut-list.c',
  'ut-list-input-stream.c',
  'ut-local-file.c',
  'ut-map.c',
  'ut-map-item.c',
  'ut-null.c',
  'ut-object.c',
  'ut-object-array.c',
  'ut-object-list.c',
  'ut-output-stream.c',
  'ut-shared-memory-array.c',
  'ut-shared-memory-subarray.c',
  'ut-string.c',
  'ut-string-array.c',
  'ut-string-list.c',
  'ut-tcp-client.c',
  'ut-uint8.c',
  'ut-uint8-array.c',
  'ut-uint8-array-with-fds.c',
  'ut-uint8-list.c',
  'ut-uint8-subarray.c',
  'ut-uint16.c',
  'ut-uint16-array.c',
  'ut-uint16-list.c',
  'ut-uint16-subarray.c',
  'ut-uint32.c',
  'ut-uint32-array.c',
  'ut-uint32-list.c',
  'ut-uint32-subarray.c',
  'ut-uint64.c',
  'ut-uint64-array.c',
  'ut-uint64-list.c',
  'ut-uint64-subarray.c',
  'ut-unix-domain-socket-client.c',
  'ut-utf8-decoder.c',
  'ut-utf8-encoder.c',
  'ut-utf8-string.c',
  'ut-writable-input-stream.c',
  'x11/ut-x11-atom-error.c',
  'x11/ut-x11-buffer.c',
  'x11/ut-x11-button-press.c',
  'x11/ut-x11-button-release.c',
  'x11/ut-x11-client.c',
  'x11/ut-x11-configure-notify.c',
  'x11/ut-x11-drawable-error.c',
  'x11/ut-x11-enter-notify.c',
  'x11/ut-x11-error.c',
  'x11/ut-x11-event.c',
  'x11/ut-x11-expose.c',
  'x11/ut-x11-extension.c',
  'x11/ut-x11-generic-event-extension.c',
  'x11/ut-x11-id-choice-error.c',
  'x11/ut-x11-implementation-error.c',
  'x11/ut-x11-key-press.c',
  'x11/ut-x11-key-release.c',
  'x11/ut-x11-leave-notify.c',
  'x11/ut-x11-length-error.c',
  'x11/ut-x11-match-error.c',
  'x11/ut-x11-mit-shm-extension.c',
  'x11/ut-x11-motion-notify.c',
  'x11/ut-x11-name-error.c',
  'x11/ut-x11-pixmap-error.c',
  'x11/ut-x11-present-extension.c',
  'x11/ut-x11-property-notify.c',
  'x11/ut-x11-request-error.c',
  'x11/ut-x11-unknown-error.c',
  'x11/ut-x11-unknown-event.c',
  'x11/ut-x11-unknown-generic-event.c',
  'x11/ut-x11-value-error.c',
  'x11/ut-x11-window-error.c',
  'xml/ut-xml-document.c',
  'xml/ut-xml-element.c',
  'zlib/ut-zlib-decoder.c',
  'zlib/ut-zlib-error.c'
]

ut_lib = static_library('ut', ut_sources, dependencies: [thread_dep, rt_dep])

cstring_test = executable('ut-cstring-test',
                          'ut-cstring-test.c',
                          link_with: ut_lib)
test('C String', cstring_test)

uint8_array_test = executable('ut-uint8-array-test',
                              'ut-uint8-array-test.c',
                              link_with: ut_lib)
test('Uint8 Array', uint8_array_test)

map_test = executable('ut-map-test',
                      'ut-map-test.c',
                      link_with: ut_lib)
test('Map', map_test)

utf8_string_test = executable('ut-utf8-string-test',
                              'ut-utf8-string-test.c',
                              link_with: ut_lib)
test('String', utf8_string_test)

const_utf8_string_test = executable('ut-constant-utf8-string-test',
                                    'ut-constant-utf8-string-test.c',
                                    link_with: ut_lib)
test('Constant String', const_utf8_string_test)

shared_memory_array_test = executable('ut-shared-memory-array-test',
                              'ut-shared-memory-array-test.c',
                              link_with: ut_lib)
test('Shared Memory Array', shared_memory_array_test)

base64_test = executable('ut-base64-test',
                         'ut-base64-test.c',
                         link_with: ut_lib)
test('Base64', base64_test)

utf8_decoder_test = executable('ut-utf8-decoder-test',
                               'ut-utf8-decoder-test.c',
                               link_with: ut_lib)
test('UTF-8 Decoder', utf8_decoder_test)

utf8_encoder_test = executable('ut-utf8-encoder-test',
                               'ut-utf8-encoder-test.c',
                               link_with: ut_lib)
test('UTF-8 Encoder', utf8_encoder_test)

xml_document_test = executable('ut-xml-document-test',
                               'xml/ut-xml-document-test.c',
                               link_with: ut_lib)
test('XML', xml_document_test)

json_test = executable('ut-json-test',
                       'json/ut-json-test.c',
                       link_with: ut_lib)
test('JSON', json_test)

deflate_decoder_test = executable('ut-deflate-decoder-test',
                                  'deflate/ut-deflate-decoder-test.c',
                                  link_with: ut_lib)
test('Deflate', deflate_decoder_test)

zlib_decoder_test = executable('ut-zlib-decoder-test',
                               'zlib/ut-zlib-decoder-test.c',
                               link_with: ut_lib)
test('zlib', zlib_decoder_test)

gzip_decoder_test = executable('ut-gzip-decoder-test',
                               'gzip/ut-gzip-decoder-test.c',
                               link_with: ut_lib)
test('GZip', gzip_decoder_test)

png_decoder_test = executable('ut-png-decoder-test',
                              'png/ut-png-decoder-test.c',
                              link_with: ut_lib)
test('PNG', png_decoder_test)

event_loop_test = executable('ut-event-loop-test',
                             'ut-event-loop-test.c',
                             link_with: ut_lib)
#test('Event Loop', event_loop_test)

local_file_test = executable('ut-local-file-test',
                             'ut-local-file-test.c',
                             link_with: ut_lib)
#test('Local File', local_file_test)

tcp_client_test = executable('ut-tcp-client-test',
                             'ut-tcp-client-test.c',
                             link_with: ut_lib)
#test('TCP Client', tcp_client_test)

unix_domain_socket_client_test = executable('ut-unix-domain-socket-client-test',
                                            'ut-unix-domain-socket-client-test.c',
                                            link_with: ut_lib)
#test('Unix Domain Socket Client', unix_domain_socket_client_test)

http_client_test = executable('ut-http-client-test',
                              'http/ut-http-client-test.c',
                              link_with: ut_lib)
#test('HTTP Client', http_client_test)

x11_client_test = executable('ut-x11-client-test',
                             'x11/ut-x11-client-test.c',
                             link_with: ut_lib)
#test('X11 Client', x11_client_test)

dbus_message_encoder_test = executable('ut-dbus-message-encoder-test',
                                       'dbus/ut-dbus-message-encoder-test.c',
				       link_with: ut_lib)
test('DBus Message Encoder', dbus_message_encoder_test)

dbus_client_test = executable('ut-dbus-client-test',
                             'dbus/ut-dbus-client-test.c',
                             link_with: ut_lib)
#test('DBus Client', dbus_client_test)
