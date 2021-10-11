#include <stdint.h>
#include <stdio.h>

#include "ut-list.h"
#include "ut-mutable-bytes.h"
#include "ut-uint8-list.h"

int main(int argc, char **argv) {
  UtObject *bytes = ut_mutable_bytes_new();
  ut_mutable_bytes_append(bytes, 0x01);
  ut_mutable_bytes_append(bytes, 0x02);
  ut_mutable_bytes_append(bytes, 0x03);
  printf("%zi\n", ut_list_get_length(bytes));
  printf("%02X\n", ut_uint8_list_get_data(bytes)[1]);
  ut_object_unref(bytes);

  return 0;
}
