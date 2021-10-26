#include <assert.h>
#include <string.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef empty = ut_string_new("");
  assert(strcmp(ut_string_get_text(empty), "") == 0);

  UtObjectRef emoji = ut_string_new("😀");
  assert(strcmp(ut_string_get_text(emoji), "😀") == 0);

  UtObjectRef string = ut_string_new("Hello World!");
  assert(strcmp(ut_string_get_text(string), "Hello World!") == 0);

  UtObjectRef cleared = ut_string_new("Hello World!");
  ut_string_clear(cleared);
  assert(strcmp(ut_string_get_text(cleared), "") == 0);

  UtObjectRef prepended1 = ut_string_new("");
  ut_string_prepend(prepended1, "Hello World!");
  assert(strcmp(ut_string_get_text(prepended1), "Hello World!") == 0);

  UtObjectRef prepended2 = ut_string_new("World!");
  ut_string_prepend(prepended2, "Hello ");
  assert(strcmp(ut_string_get_text(prepended2), "Hello World!") == 0);

  UtObjectRef prepended3 = ut_string_new("");
  ut_string_prepend_code_point(prepended3, 'A');
  assert(strcmp(ut_string_get_text(prepended3), "A") == 0);

  UtObjectRef prepended4 = ut_string_new("BC");
  ut_string_prepend_code_point(prepended4, 'A');
  assert(strcmp(ut_string_get_text(prepended4), "ABC") == 0);

  UtObjectRef prepended5 = ut_string_new("");
  ut_string_prepend_code_point(prepended5, 0x1f600);
  assert(strcmp(ut_string_get_text(prepended5), "😀") == 0);

  UtObjectRef prepended6 = ut_string_new(" Smile!");
  ut_string_prepend_code_point(prepended6, 0x1f600);
  assert(strcmp(ut_string_get_text(prepended6), "😀 Smile!") == 0);

  UtObjectRef appended1 = ut_string_new("");
  ut_string_append(appended1, "Hello World!");
  assert(strcmp(ut_string_get_text(appended1), "Hello World!") == 0);

  UtObjectRef appended2 = ut_string_new("Hello");
  ut_string_append(appended2, " World!");
  assert(strcmp(ut_string_get_text(appended2), "Hello World!") == 0);

  UtObjectRef appended3 = ut_string_new("");
  ut_string_append_code_point(appended3, 'C');
  assert(strcmp(ut_string_get_text(appended3), "C") == 0);

  UtObjectRef appended4 = ut_string_new("AB");
  ut_string_append_code_point(appended4, 'C');
  assert(strcmp(ut_string_get_text(appended4), "ABC") == 0);

  UtObjectRef appended5 = ut_string_new("");
  ut_string_append_code_point(appended5, 0x1f600);
  assert(strcmp(ut_string_get_text(appended5), "😀") == 0);

  UtObjectRef appended6 = ut_string_new("Smile! ");
  ut_string_append_code_point(appended6, 0x1f600);
  assert(strcmp(ut_string_get_text(appended6), "Smile! 😀") == 0);

  UtObjectRef encoding = ut_string_new("$¢€𐐷😀");
  UtObjectRef code_points = ut_string_get_code_points(encoding);
  assert(ut_list_get_length(code_points) == 5);
  assert(ut_uint32_list_get_element(code_points, 0) == 0x24);
  assert(ut_uint32_list_get_element(code_points, 1) == 0xa2);
  assert(ut_uint32_list_get_element(code_points, 2) == 0x20ac);
  assert(ut_uint32_list_get_element(code_points, 3) == 0x10437);
  assert(ut_uint32_list_get_element(code_points, 4) == 0x1f600);
  UtObjectRef utf8 = ut_string_get_utf8(encoding);
  assert(ut_list_get_length(utf8) == 14);
  // $
  assert(ut_uint8_list_get_element(utf8, 0) == 0x24);
  // ¢
  assert(ut_uint8_list_get_element(utf8, 1) == 0xc2);
  assert(ut_uint8_list_get_element(utf8, 2) == 0xa2);
  // €
  assert(ut_uint8_list_get_element(utf8, 3) == 0xe2);
  assert(ut_uint8_list_get_element(utf8, 4) == 0x82);
  assert(ut_uint8_list_get_element(utf8, 5) == 0xac);
  // 𐐷
  assert(ut_uint8_list_get_element(utf8, 6) == 0xf0);
  assert(ut_uint8_list_get_element(utf8, 7) == 0x90);
  assert(ut_uint8_list_get_element(utf8, 8) == 0x90);
  assert(ut_uint8_list_get_element(utf8, 9) == 0xb7);
  // 😀
  assert(ut_uint8_list_get_element(utf8, 10) == 0xf0);
  assert(ut_uint8_list_get_element(utf8, 11) == 0x9f);
  assert(ut_uint8_list_get_element(utf8, 12) == 0x98);
  assert(ut_uint8_list_get_element(utf8, 13) == 0x80);
  UtObjectRef utf16 = ut_string_get_utf16(encoding);
  assert(ut_list_get_length(utf16) == 7);
  // $
  assert(ut_uint16_list_get_element(utf16, 0) == 0x24);
  // ¢
  assert(ut_uint16_list_get_element(utf16, 1) == 0xa2);
  // €
  assert(ut_uint16_list_get_element(utf16, 2) == 0x20ac);
  // 𐐷
  assert(ut_uint16_list_get_element(utf16, 3) == 0xd801);
  assert(ut_uint16_list_get_element(utf16, 4) == 0xdc37);
  // 😀
  assert(ut_uint16_list_get_element(utf16, 5) == 0xd83d);
  assert(ut_uint16_list_get_element(utf16, 6) == 0xde00);

  return 0;
}
