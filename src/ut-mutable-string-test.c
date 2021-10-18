#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObject *string2 = ut_mutable_string_new(" ");
  printf("'%s'\n", ut_string_get_text(string2));
  ut_mutable_string_prepend(string2, "Hello");
  printf("'%s'\n", ut_string_get_text(string2));
  ut_mutable_string_append(string2, "World!");
  printf("'%s'\n", ut_string_get_text(string2));

  return 0;
}
