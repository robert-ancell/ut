#include <assert.h>
#include <string.h>

#include "ut.h"

int main(int argc, char **argv) {
  ut_cstring_ref printf_value = ut_cstring_new_printf("Number %d", 1);
  ut_assert_cstring_equal(printf_value, "Number 1");

  ut_cstring_ref prefix_value = ut_cstring_new("Food");
  assert(ut_cstring_starts_with(prefix_value, "Foo"));
  assert(!ut_cstring_starts_with(prefix_value, "Bar"));
  assert(!ut_cstring_starts_with(prefix_value, "For"));

  ut_cstring_ref suffix_value = ut_cstring_new("Phone");
  assert(ut_cstring_ends_with(suffix_value, "one"));
  assert(!ut_cstring_ends_with(suffix_value, "two"));
  assert(!ut_cstring_ends_with(suffix_value, "ine"));

  ut_cstring_ref join1_value =
      ut_cstring_join(",", "one", "two", "three", NULL);
  ut_assert_cstring_equal(join1_value, "one,two,three");

  ut_cstring_ref join2_value = ut_cstring_join(",", "one", NULL);
  ut_assert_cstring_equal(join2_value, "one");

  ut_cstring_ref join3_value = ut_cstring_join(",", NULL);
  ut_assert_cstring_equal(join3_value, "");

  ut_cstring_ref substring1_value = ut_cstring_substring("World", 0, 5);
  ut_assert_cstring_equal(substring1_value, "World");

  ut_cstring_ref substring2_value = ut_cstring_substring("World", 1, 5);
  ut_assert_cstring_equal(substring2_value, "orld");

  ut_cstring_ref substring3_value = ut_cstring_substring("World", 0, 4);
  ut_assert_cstring_equal(substring3_value, "Worl");

  ut_cstring_ref substring4_value = ut_cstring_substring("World", 1, 4);
  ut_assert_cstring_equal(substring4_value, "orl");

  return 0;
}
