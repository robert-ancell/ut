#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef list = ut_uint8_array_new_with_data(3, 0x01, 0x02, 0x03);
  printf("%zi\n", ut_list_get_length(list));
  printf("%02X\n", ut_uint8_array_get_data(list)[1]);
  printf("%s\n", ut_object_get_type_name(list));
  ut_cstring list_string = ut_object_to_string(list);
  printf("list: %s\n", list_string);

  return 0;
}
