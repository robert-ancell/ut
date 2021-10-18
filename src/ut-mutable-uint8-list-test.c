#include <stdio.h>
#include <stdlib.h>

#include "ut.h"

int main(int argc, char **argv) {
  UtObject *list = ut_mutable_uint8_list_new();
  ut_mutable_uint8_list_append(list, 0x01);
  ut_mutable_uint8_list_append(list, 0x02);
  ut_mutable_uint8_list_append(list, 0x03);
  printf("%zi\n", ut_list_get_length(list));
  printf("%02X\n", ut_uint8_list_get_data(list)[1]);
  printf("%s\n", ut_object_get_type_name(list));
  char *list_string = ut_object_to_string(list);
  printf("list: %s\n", list_string);
  free(list_string);
  ut_object_unref(list);

  return 0;
}
