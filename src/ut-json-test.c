#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut.h"

static void test_encode() {
  UtObjectRef null = ut_null_new();
  char *null_text = ut_json_encode(null);
  assert(strcmp(null_text, "null") == 0);
  free(null_text);

  UtObjectRef boolean_true = ut_boolean_new(true);
  char *boolean_true_text = ut_json_encode(boolean_true);
  assert(strcmp(boolean_true_text, "true") == 0);
  free(boolean_true_text);

  UtObjectRef boolean_false = ut_boolean_new(false);
  char *boolean_false_text = ut_json_encode(boolean_false);
  assert(strcmp(boolean_false_text, "false") == 0);
  free(boolean_false_text);

  UtObjectRef zero = ut_int64_new(0);
  char *zero_text = ut_json_encode(zero);
  assert(strcmp(zero_text, "0") == 0);
  free(zero_text);

  UtObjectRef one = ut_int64_new(1);
  char *one_text = ut_json_encode(one);
  assert(strcmp(one_text, "1") == 0);
  free(one_text);

  UtObjectRef minus_one = ut_int64_new(-1);
  char *minus_one_text = ut_json_encode(minus_one);
  assert(strcmp(minus_one_text, "-1") == 0);
  free(minus_one_text);

  UtObjectRef one_k = ut_int64_new(1024);
  char *one_k_text = ut_json_encode(one_k);
  assert(strcmp(one_k_text, "1024") == 0);
  free(one_k_text);

  // FIXME: float

  UtObjectRef empty_string = ut_string_new("");
  char *empty_string_text = ut_json_encode(empty_string);
  assert(strcmp(empty_string_text, "\"\"") == 0);
  free(empty_string_text);

  UtObjectRef string = ut_string_new("Hello World!");
  char *string_text = ut_json_encode(string);
  assert(strcmp(string_text, "\"Hello World!\"") == 0);
  free(string_text);

  UtObjectRef escaped_string = ut_string_new("\"\\/\b\f\n\r\t\x12");
  char *escaped_string_text = ut_json_encode(escaped_string);
  assert(strcmp(escaped_string_text, "\"\\\"\\\\/\\b\\f\\n\\r\\t\\u0012\"") ==
         0);
  free(escaped_string_text);

  UtObjectRef emoji_string = ut_string_new("😀");
  char *emoji_string_text = ut_json_encode(emoji_string);
  assert(strcmp(emoji_string_text, "\"😀\"") == 0);
  free(emoji_string_text);

  UtObjectRef empty_array = ut_list_new();
  char *empty_array_text = ut_json_encode(empty_array);
  assert(strcmp(empty_array_text, "[]") == 0);
  free(empty_array_text);

  UtObjectRef number_array = ut_list_new();
  ut_mutable_list_append_take(number_array, ut_int64_new(1));
  ut_mutable_list_append_take(number_array, ut_int64_new(2));
  ut_mutable_list_append_take(number_array, ut_int64_new(3));
  char *number_array_text = ut_json_encode(number_array);
  assert(strcmp(number_array_text, "[1,2,3]") == 0);
  free(number_array_text);

  UtObjectRef mixed_array = ut_list_new();
  ut_mutable_list_append_take(mixed_array, ut_boolean_new(false));
  ut_mutable_list_append_take(mixed_array, ut_string_new("two"));
  ut_mutable_list_append_take(mixed_array, ut_float64_new(3.1));
  char *mixed_array_text = ut_json_encode(mixed_array);
  assert(strcmp(mixed_array_text, "[false,\"two\",3.1]") == 0);
  free(mixed_array_text);

  UtObjectRef empty_object = ut_map_new();
  char *empty_object_text = ut_json_encode(empty_object);
  assert(strcmp(empty_object_text, "{}") == 0);
  free(empty_object_text);

  UtObjectRef number_object = ut_map_new();
  ut_map_insert_take(number_object, ut_string_new("one"), ut_int64_new(1));
  ut_map_insert_take(number_object, ut_string_new("two"), ut_int64_new(2));
  ut_map_insert_take(number_object, ut_string_new("three"), ut_int64_new(3));
  char *number_object_text = ut_json_encode(number_object);
  assert(strcmp(number_object_text, "{\"one\":1,\"two\":2,\"three\":3}") == 0);
  free(number_object_text);
}

static void test_decode() {
  UtObjectRef empty = ut_json_decode("");
  assert(empty == NULL);

  UtObjectRef unknown_keyword = ut_json_decode("foo");
  assert(unknown_keyword == NULL);

  UtObjectRef unknown_container = ut_json_decode("<>");
  assert(unknown_container == NULL);

  UtObjectRef null = ut_json_decode("null");
  assert(null != NULL);
  assert(ut_object_is_null(null));

  UtObjectRef true_keyword = ut_json_decode("true");
  assert(true_keyword != NULL);
  assert(ut_object_is_boolean(true_keyword));
  assert(ut_boolean_get_value(true_keyword) == true);

  UtObjectRef false_keyword = ut_json_decode("false");
  assert(false_keyword != NULL);
  assert(ut_object_is_boolean(false_keyword));
  assert(ut_boolean_get_value(false_keyword) == false);

  UtObjectRef zero = ut_json_decode("0");
  assert(zero != NULL);
  assert(ut_object_is_int64(zero));
  assert(ut_int64_get_value(zero) == 0);

  UtObjectRef one = ut_json_decode("1");
  assert(one != NULL);
  assert(ut_object_is_int64(one));
  assert(ut_int64_get_value(one) == 1);

  UtObjectRef minus_one = ut_json_decode("-1");
  assert(minus_one != NULL);
  assert(ut_object_is_int64(minus_one));
  assert(ut_int64_get_value(minus_one) == -1);

  UtObjectRef one_k = ut_json_decode("1024");
  assert(one_k != NULL);
  assert(ut_object_is_int64(one_k));
  assert(ut_int64_get_value(one_k) == 1024);

  UtObjectRef one_point_one = ut_json_decode("1.1");
  assert(one_point_one != NULL);
  assert(ut_object_is_float64(one_point_one));
  assert(ut_float64_get_value(one_point_one) == 1.1);

  UtObjectRef minus_one_point_one = ut_json_decode("-1.1");
  assert(minus_one_point_one != NULL);
  assert(ut_object_is_float64(minus_one_point_one));
  assert(ut_float64_get_value(minus_one_point_one) == -1.1);

  UtObjectRef scientific_number = ut_json_decode("1.024e3");
  assert(scientific_number != NULL);
  assert(ut_object_is_float64(scientific_number));
  assert(ut_float64_get_value(scientific_number) == 1024.0);

  UtObjectRef one_M = ut_json_decode("1e6");
  assert(one_M != NULL);
  assert(ut_object_is_float64(one_M));
  assert(ut_float64_get_value(one_M) == 1000000);

  UtObjectRef one_u = ut_json_decode("1e-6");
  assert(one_u != NULL);
  assert(ut_object_is_float64(one_u));
  assert(ut_float64_get_value(one_u) == 0.000001);

  UtObjectRef empty_string = ut_json_decode("\"\"");
  assert(empty_string != NULL);
  assert(ut_object_implements_string(empty_string));
  assert(strcmp(ut_string_get_text(empty_string), "") == 0);

  UtObjectRef string = ut_json_decode("\"Hello World!\"");
  assert(string != NULL);
  assert(ut_object_implements_string(string));
  assert(strcmp(ut_string_get_text(string), "Hello World!") == 0);

  UtObjectRef escaped_string =
      ut_json_decode("\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0012\"");
  assert(escaped_string != NULL);
  assert(ut_object_implements_string(escaped_string));
  assert(strcmp(ut_string_get_text(escaped_string), "\"\\/\b\f\n\r\t\x12") ==
         0);

  UtObjectRef emoji_string = ut_json_decode("\"😀\"");
  assert(emoji_string != NULL);
  assert(ut_object_implements_string(emoji_string));
  assert(strcmp(ut_string_get_text(emoji_string), "😀") == 0);

  UtObjectRef unterminated_array = ut_json_decode("[");
  assert(unterminated_array == NULL);

  UtObjectRef empty_array = ut_json_decode("[]");
  assert(empty_array != NULL);
  assert(ut_object_implements_list(empty_array));
  assert(ut_list_get_length(empty_array) == 0);

  UtObjectRef unterminated_object = ut_json_decode("{");
  assert(unterminated_object == NULL);

  UtObjectRef empty_object = ut_json_decode("{}");
  assert(empty_object != NULL);
  assert(ut_object_implements_map(empty_object));
  assert(ut_map_get_length(empty_object) == 0);
}

int main(int argc, char **argv) {
  test_encode();
  test_decode();
}
