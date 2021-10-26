#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut.h"

static void test_encode() {
  UtObjectRef empty_list = ut_uint8_array_new();
  ut_cstring empty_text = ut_base64_encode(empty_list);
  assert(strcmp(empty_text, "") == 0);

  UtObjectRef short1_list = ut_uint8_array_new_with_data(1, 'M');
  ut_cstring short1_text = ut_base64_encode(short1_list);
  assert(strcmp(short1_text, "TQ==") == 0);

  UtObjectRef short2_list = ut_uint8_array_new_with_data(2, 'M', 'a');
  ut_cstring short2_text = ut_base64_encode(short2_list);
  assert(strcmp(short2_text, "TWE=") == 0);

  UtObjectRef short3_list = ut_uint8_array_new_with_data(3, 'M', 'a', 'n');
  ut_cstring short3_text = ut_base64_encode(short3_list);
  assert(strcmp(short3_text, "TWFu") == 0);

  UtObjectRef sentence = ut_string_new("Many hands make light work.");
  UtObjectRef sentence_list = ut_string_get_utf8(sentence);
  ut_cstring sentence_text = ut_base64_encode(sentence_list);
  assert(strcmp(sentence_text, "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu") == 0);

  UtObjectRef binary_list = ut_uint8_list_new();
  for (size_t i = 0; i < 256; i++) {
    ut_uint8_list_append(binary_list, i);
  }
  ut_cstring binary_text = ut_base64_encode(binary_list);
  assert(
      strcmp(binary_text,
             "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vM"
             "DEyMzQ1Njc4OTo7PD0+"
             "P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub"
             "3BxcnN0dXZ3eHl6e3x9fn+"
             "AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+"
             "wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/"
             "g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==") == 0);
}

static void test_decode() {
  UtObjectRef empty = ut_base64_decode("");
  assert(empty != NULL);
  assert(ut_object_implements_uint8_list(empty));
  assert(ut_list_get_length(empty) == 0);

  UtObjectRef whitespace = ut_base64_decode(" ");
  assert(whitespace != NULL);
  assert(ut_object_implements_error(whitespace));

  UtObjectRef unknown_character = ut_base64_decode("!");
  assert(unknown_character != NULL);
  assert(ut_object_implements_error(unknown_character));

  UtObjectRef non_ascii_character = ut_base64_decode("\xff");
  assert(non_ascii_character != NULL);
  assert(ut_object_implements_error(non_ascii_character));

  UtObjectRef missing_data = ut_base64_decode("T");
  assert(missing_data != NULL);
  assert(ut_object_implements_error(missing_data));

  UtObjectRef short1a = ut_base64_decode("TQ==");
  assert(short1a != NULL);
  assert(ut_object_implements_uint8_list(short1a));
  assert(ut_list_get_length(short1a) == 1);

  UtObjectRef short1b = ut_base64_decode("TQ");
  assert(short1b != NULL);
  assert(ut_object_implements_uint8_list(short1b));
  assert(ut_list_get_length(short1b) == 1);

  UtObjectRef short2a = ut_base64_decode("TWE=");
  assert(short2a != NULL);
  assert(ut_object_implements_uint8_list(short2a));
  assert(ut_list_get_length(short2a) == 2);

  UtObjectRef short2b = ut_base64_decode("TWE");
  assert(short2b != NULL);
  assert(ut_object_implements_uint8_list(short2b));
  assert(ut_list_get_length(short2b) == 2);

  UtObjectRef short3 = ut_base64_decode("TQFu");
  assert(short3 != NULL);
  assert(ut_object_implements_uint8_list(short3));
  assert(ut_list_get_length(short3) == 3);

  UtObjectRef sentence =
      ut_base64_decode("TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu");
  assert(sentence != NULL);
  assert(ut_object_implements_uint8_list(sentence));
  UtObjectRef sentence_text = ut_utf8_string_new_from_data(sentence);
  assert(strcmp(ut_string_get_text(sentence_text),
                "Many hands make light work.") == 0);

  UtObjectRef binary = ut_base64_decode(
      "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1"
      "Njc4OTo7PD0+"
      "P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0"
      "dXZ3eHl6e3x9fn+"
      "AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+"
      "wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/"
      "g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==");
  assert(binary != NULL);
  assert(ut_object_implements_uint8_list(binary));
  assert(ut_list_get_length(binary) == 256);
  for (size_t i = 0; i < 256; i++) {
    assert(ut_uint8_list_get_element(binary, i) == i);
  }
}

int main(int argc, char **argv) {
  test_encode();
  test_decode();
}
