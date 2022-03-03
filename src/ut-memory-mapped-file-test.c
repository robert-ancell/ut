#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef file = ut_memory_mapped_file_new("README.md");
  ut_file_open_read(file);
  printf("%zi\n", ut_list_get_length(file));
  ut_cstring_ref list_string = ut_list_to_string(file);
  printf("list: %s\n", list_string);

  return 0;
}
