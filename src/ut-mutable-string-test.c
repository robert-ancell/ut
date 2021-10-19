#include <assert.h>
#include <string.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObject *empty = ut_mutable_string_new("");
  assert(strcmp(ut_string_get_text(empty), "") == 0);
  ut_object_unref(empty);

  UtObject *emoji = ut_mutable_string_new("😀");
  assert(strcmp(ut_string_get_text(emoji), "😀") == 0);
  ut_object_unref(emoji);

  UtObject *string = ut_mutable_string_new("Hello World!");
  assert(strcmp(ut_string_get_text(string), "Hello World!") == 0);
  ut_object_unref(string);

  UtObject *cleared = ut_mutable_string_new("Hello World!");
  ut_mutable_string_clear(cleared);
  assert(strcmp(ut_string_get_text(cleared), "") == 0);
  ut_object_unref(cleared);

  UtObject *prepended1 = ut_mutable_string_new("");
  ut_mutable_string_prepend(prepended1, "Hello World!");
  assert(strcmp(ut_string_get_text(prepended1), "Hello World!") == 0);
  ut_object_unref(prepended1);

  UtObject *prepended2 = ut_mutable_string_new("World!");
  ut_mutable_string_prepend(prepended2, "Hello ");
  assert(strcmp(ut_string_get_text(prepended2), "Hello World!") == 0);
  ut_object_unref(prepended2);

  UtObject *prepended3 = ut_mutable_string_new("");
  ut_mutable_string_prepend_code_point(prepended3, 'A');
  assert(strcmp(ut_string_get_text(prepended3), "A") == 0);
  ut_object_unref(prepended3);

  UtObject *prepended4 = ut_mutable_string_new("BC");
  ut_mutable_string_prepend_code_point(prepended4, 'A');
  assert(strcmp(ut_string_get_text(prepended4), "ABC") == 0);
  ut_object_unref(prepended4);

  UtObject *prepended5 = ut_mutable_string_new("");
  ut_mutable_string_prepend_code_point(prepended5, 0x1f600);
  assert(strcmp(ut_string_get_text(prepended5), "😀") == 0);
  ut_object_unref(prepended5);

  UtObject *prepended6 = ut_mutable_string_new(" Smile!");
  ut_mutable_string_prepend_code_point(prepended6, 0x1f600);
  assert(strcmp(ut_string_get_text(prepended6), "😀 Smile!") == 0);
  ut_object_unref(prepended6);

  UtObject *appended1 = ut_mutable_string_new("");
  ut_mutable_string_append(appended1, "Hello World!");
  assert(strcmp(ut_string_get_text(appended1), "Hello World!") == 0);
  ut_object_unref(appended1);

  UtObject *appended2 = ut_mutable_string_new("Hello");
  ut_mutable_string_append(appended2, " World!");
  assert(strcmp(ut_string_get_text(appended2), "Hello World!") == 0);
  ut_object_unref(appended2);

  UtObject *appended3 = ut_mutable_string_new("");
  ut_mutable_string_append_code_point(appended3, 'C');
  assert(strcmp(ut_string_get_text(appended3), "C") == 0);
  ut_object_unref(appended3);

  UtObject *appended4 = ut_mutable_string_new("AB");
  ut_mutable_string_append_code_point(appended4, 'C');
  assert(strcmp(ut_string_get_text(appended4), "ABC") == 0);
  ut_object_unref(appended4);

  UtObject *appended5 = ut_mutable_string_new("");
  ut_mutable_string_append_code_point(appended5, 0x1f600);
  assert(strcmp(ut_string_get_text(appended5), "😀") == 0);
  ut_object_unref(appended5);

  UtObject *appended6 = ut_mutable_string_new("Smile! ");
  ut_mutable_string_append_code_point(appended6, 0x1f600);
  assert(strcmp(ut_string_get_text(appended6), "Smile! 😀") == 0);
  ut_object_unref(appended6);

  return 0;
}
