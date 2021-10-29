#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut.h"

static void test_encode() {
  UtObjectRef null = ut_null_new();
  ut_cstring null_text = ut_json_encode(null);
  ut_assert_cstring_equal(null_text, "null");

  UtObjectRef boolean_true = ut_boolean_new(true);
  ut_cstring boolean_true_text = ut_json_encode(boolean_true);
  ut_assert_cstring_equal(boolean_true_text, "true");

  UtObjectRef boolean_false = ut_boolean_new(false);
  ut_cstring boolean_false_text = ut_json_encode(boolean_false);
  ut_assert_cstring_equal(boolean_false_text, "false");

  UtObjectRef zero = ut_int64_new(0);
  ut_cstring zero_text = ut_json_encode(zero);
  ut_assert_cstring_equal(zero_text, "0");

  UtObjectRef one = ut_int64_new(1);
  ut_cstring one_text = ut_json_encode(one);
  ut_assert_cstring_equal(one_text, "1");

  UtObjectRef minus_one = ut_int64_new(-1);
  ut_cstring minus_one_text = ut_json_encode(minus_one);
  ut_assert_cstring_equal(minus_one_text, "-1");

  UtObjectRef one_k = ut_int64_new(1024);
  ut_cstring one_k_text = ut_json_encode(one_k);
  ut_assert_cstring_equal(one_k_text, "1024");

  UtObjectRef one_point_one = ut_float64_new(1.1);
  ut_cstring one_point_one_text = ut_json_encode(one_point_one);
  ut_assert_cstring_equal(one_point_one_text, "1.100000e+00");

  UtObjectRef minus_one_point_one = ut_float64_new(-1.1);
  ut_cstring minus_one_point_one_text = ut_json_encode(minus_one_point_one);
  ut_assert_cstring_equal(minus_one_point_one_text, "-1.100000e+00");

  UtObjectRef scientific_number = ut_float64_new(1024);
  ut_cstring scientific_number_text = ut_json_encode(scientific_number);
  ut_assert_cstring_equal(scientific_number_text, "1.024000e+03");

  UtObjectRef one_M = ut_float64_new(1000000);
  ut_cstring one_M_text = ut_json_encode(one_M);
  ut_assert_cstring_equal(one_M_text, "1.000000e+06");

  UtObjectRef one_u = ut_float64_new(0.000001);
  ut_cstring one_u_text = ut_json_encode(one_u);
  ut_assert_cstring_equal(one_u_text, "1.000000e-06");

  UtObjectRef empty_string = ut_string_new("");
  ut_cstring empty_string_text = ut_json_encode(empty_string);
  ut_assert_cstring_equal(empty_string_text, "\"\"");

  UtObjectRef string = ut_string_new("Hello World!");
  ut_cstring string_text = ut_json_encode(string);
  ut_assert_cstring_equal(string_text, "\"Hello World!\"");

  UtObjectRef escaped_string = ut_string_new("\"\\/\b\f\n\r\t\x12");
  ut_cstring escaped_string_text = ut_json_encode(escaped_string);
  ut_assert_cstring_equal(escaped_string_text,
                          "\"\\\"\\\\/\\b\\f\\n\\r\\t\\u0012\"");

  UtObjectRef emoji_string = ut_string_new("😀");
  ut_cstring emoji_string_text = ut_json_encode(emoji_string);
  ut_assert_cstring_equal(emoji_string_text, "\"😀\"");

  UtObjectRef empty_array = ut_list_new();
  ut_cstring empty_array_text = ut_json_encode(empty_array);
  ut_assert_cstring_equal(empty_array_text, "[]");

  UtObjectRef number_array = ut_list_new();
  ut_list_append_take(number_array, ut_int64_new(1));
  ut_list_append_take(number_array, ut_int64_new(2));
  ut_list_append_take(number_array, ut_int64_new(3));
  ut_cstring number_array_text = ut_json_encode(number_array);
  ut_assert_cstring_equal(number_array_text, "[1,2,3]");

  UtObjectRef mixed_array = ut_list_new();
  ut_list_append_take(mixed_array, ut_boolean_new(false));
  ut_list_append_take(mixed_array, ut_string_new("two"));
  ut_list_append_take(mixed_array, ut_float64_new(3.1));
  ut_cstring mixed_array_text = ut_json_encode(mixed_array);
  ut_assert_cstring_equal(mixed_array_text, "[false,\"two\",3.100000e+00]");

  UtObjectRef empty_object = ut_map_new();
  ut_cstring empty_object_text = ut_json_encode(empty_object);
  ut_assert_cstring_equal(empty_object_text, "{}");

  UtObjectRef number_object = ut_map_new();
  ut_map_insert_take(number_object, ut_string_new("one"), ut_int64_new(1));
  ut_map_insert_take(number_object, ut_string_new("two"), ut_int64_new(2));
  ut_map_insert_take(number_object, ut_string_new("three"), ut_int64_new(3));
  ut_cstring number_object_text = ut_json_encode(number_object);
  ut_assert_cstring_equal(number_object_text,
                          "{\"one\":1,\"two\":2,\"three\":3}");

  UtObjectRef mixed_object = ut_map_new();
  ut_map_insert_take(mixed_object, ut_string_new("boolean"),
                     ut_boolean_new(true));
  ut_map_insert_take(mixed_object, ut_string_new("number"), ut_int64_new(42));
  ut_map_insert_take(mixed_object, ut_string_new("string"),
                     ut_string_new("foo"));
  ut_cstring mixed_object_text = ut_json_encode(mixed_object);
  ut_assert_cstring_equal(
      mixed_object_text, "{\"boolean\":true,\"number\":42,\"string\":\"foo\"}");
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
  ut_assert_cstring_equal(ut_string_get_text(empty_string), "");

  UtObjectRef string = ut_json_decode("\"Hello World!\"");
  assert(string != NULL);
  assert(ut_object_implements_string(string));
  ut_assert_cstring_equal(ut_string_get_text(string), "Hello World!");

  UtObjectRef escaped_string =
      ut_json_decode("\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\u0012\"");
  assert(escaped_string != NULL);
  assert(ut_object_implements_string(escaped_string));
  ut_assert_cstring_equal(ut_string_get_text(escaped_string),
                          "\"\\/\b\f\n\r\t\x12");

  UtObjectRef emoji_string = ut_json_decode("\"😀\"");
  assert(emoji_string != NULL);
  assert(ut_object_implements_string(emoji_string));
  ut_assert_cstring_equal(ut_string_get_text(emoji_string), "😀");

  UtObjectRef unterminated_array = ut_json_decode("[");
  assert(unterminated_array == NULL);

  UtObjectRef empty_array = ut_json_decode("[]");
  assert(empty_array != NULL);
  assert(ut_object_implements_list(empty_array));
  assert(ut_list_get_length(empty_array) == 0);

  UtObjectRef number_array = ut_json_decode("[1,2,3]");
  assert(number_array != NULL);
  assert(ut_object_implements_list(number_array));
  assert(ut_list_get_length(number_array) == 3);

  UtObjectRef mixed_array = ut_json_decode("[false,\"two\",3.1]");
  assert(mixed_array != NULL);
  assert(ut_object_implements_list(mixed_array));
  assert(ut_list_get_length(mixed_array) == 3);

  UtObjectRef unterminated_object = ut_json_decode("{");
  assert(unterminated_object == NULL);

  UtObjectRef empty_object = ut_json_decode("{}");
  assert(empty_object != NULL);
  assert(ut_object_implements_map(empty_object));
  assert(ut_map_get_length(empty_object) == 0);

  UtObjectRef number_object =
      ut_json_decode("{\"one\":1,\"two\":2,\"three\":3}");
  assert(number_object != NULL);
  assert(ut_object_implements_map(number_object));
  assert(ut_map_get_length(number_object) == 3);
  UtObjectRef number_value1 = ut_map_lookup_string(number_object, "one");
  assert(number_value1 != NULL);
  assert(ut_object_is_int64(number_value1));
  assert(ut_int64_get_value(number_value1) == 1);
  UtObjectRef number_value2 = ut_map_lookup_string(number_object, "two");
  assert(number_value2 != NULL);
  assert(ut_object_is_int64(number_value2));
  assert(ut_int64_get_value(number_value2) == 2);
  UtObjectRef number_value3 = ut_map_lookup_string(number_object, "three");
  assert(number_value3 != NULL);
  assert(ut_object_is_int64(number_value3));
  assert(ut_int64_get_value(number_value3) == 3);

  UtObjectRef mixed_object =
      ut_json_decode("{\"boolean\":true,\"number\":42,\"string\":\"foo\"}");
  assert(mixed_object != NULL);
  assert(ut_object_implements_map(mixed_object));
  assert(ut_map_get_length(mixed_object) == 3);
  UtObjectRef mixed_value1 = ut_map_lookup_string(mixed_object, "boolean");
  assert(mixed_value1 != NULL);
  assert(ut_object_is_boolean(mixed_value1));
  assert(ut_boolean_get_value(mixed_value1) == true);
  UtObjectRef mixed_value2 = ut_map_lookup_string(mixed_object, "number");
  assert(mixed_value2 != NULL);
  assert(ut_object_is_int64(mixed_value2));
  assert(ut_int64_get_value(mixed_value2) == 42);
  UtObjectRef mixed_value3 = ut_map_lookup_string(mixed_object, "string");
  assert(mixed_value3 != NULL);
  assert(ut_object_implements_string(mixed_value3));
  ut_assert_cstring_equal(ut_string_get_text(mixed_value3), "foo");
}

int main(int argc, char **argv) {
  test_encode();
  test_decode();
}
