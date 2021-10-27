ut_sources = [
  'ut-base64.c',
  'ut-boolean.c',
  'ut-cancel.c',
  'ut-constant-utf8-string.c',
  'ut-constant-uint8-array.c',
  'ut-end-of-stream.c',
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
  'ut-http-client.c',
  'ut-http-header.c',
  'ut-http-response.c',
  'ut-input-stream.c',
  'ut-int64.c',
  'ut-json.c',
  'ut-list.c',
  'ut-local-file.c',
  'ut-map.c',
  'ut-map-item.c',
  'ut-null.c',
  'ut-object.c',
  'ut-object-array.c',
  'ut-object-list.c',
  'ut-output-stream.c',
  'ut-string.c',
  'ut-string-array.c',
  'ut-tcp-client.c',
  'ut-uint8.c',
  'ut-uint8-array.c',
  'ut-uint8-list.c',
  'ut-uint16.c',
  'ut-uint16-array.c',
  'ut-uint16-list.c',
  'ut-uint32.c',
  'ut-uint32-array.c',
  'ut-uint32-list.c',
  'ut-uint64.c',
  'ut-uint64-array.c',
  'ut-uint64-list.c',
  'ut-unix-domain-socket-client.c',
  'ut-utf8-decoder.c',
  'ut-utf8-encoder.c',
  'ut-utf8-string.c',
  'ut-xml-document.c',
  'ut-xml-element.c'
]

ut_lib = static_library('ut', ut_sources, dependencies: thread_dep)

uint8_array_test = executable('ut-uint8-array-test',
                              'ut-uint8-array-test.c',
                              link_with: ut_lib)
test('Uint8 Array', uint8_array_test)

map_test = executable('ut-map-test',
                      'ut-map-test.c',
                      link_with: ut_lib)
test('Map', map_test)

tcp_client_test = executable('ut-tcp-client-test',
                             'ut-tcp-client-test.c',
                             link_with: ut_lib)
utf8_string_test = executable('ut-utf8-string-test',
                              'ut-utf8-string-test.c',
                              link_with: ut_lib)
test('String', utf8_string_test)

const_utf8_string_test = executable('ut-constant-utf8-string-test',
                                    'ut-constant-utf8-string-test.c',
                                    link_with: ut_lib)
test('Constant String', const_utf8_string_test)

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
                               'ut-xml-document-test.c',
                               link_with: ut_lib)
test('XML', xml_document_test)

json_test = executable('ut-json-test',
                       'ut-json-test.c',
                       link_with: ut_lib)
test('JSON', json_test)

event_loop_test = executable('ut-event-loop-test',
                             'ut-event-loop-test.c',
                             link_with: ut_lib)
#test('Event Loop', event_loop_test)

local_file_test = executable('ut-local-file-test',
                             'ut-local-file-test.c',
                             link_with: ut_lib)
#test('Local File', local_file_test)

#test('TCP Client', tcp_client_test)

unix_domain_socket_client_test = executable('ut-unix-domain-socket-client-test',
                                            'ut-unix-domain-socket-client-test.c',
                                            link_with: ut_lib)
#test('Unix Domain Socket Client', unix_domain_socket_client_test)

http_client_test = executable('ut-http-client-test',
                              'ut-http-client-test.c',
                              link_with: ut_lib)
#test('HTTP Client', http_client_test)
