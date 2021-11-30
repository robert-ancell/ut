#include "ut.h"

int main(int argc, char **argv) {
  UtObjectRef file = ut_memory_mapped_file_new(
      "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf");
  ut_file_open_read(file);
  UtObjectRef decoder = ut_true_type_decoder_new(file);
  ut_true_type_decoder_decode(decoder);

  ut_event_loop_run();

  return 0;
}
