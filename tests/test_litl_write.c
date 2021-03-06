/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test validates the event recording mechanism
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "litl_types.h"
#include "litl_write.h"


int main(int argc, char **argv) {
  int i, nb_iter;

  litl_write_trace_t* trace;
  char* filename = "trace";
  const uint32_t buffer_size = 32 * 1024; // 32KB

  if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
    filename = argv[2];
  else
#ifdef LITL_TESTBUFFER_FLUSH
    filename = "/tmp/test_litl_write_flush.trace";
#else
    filename = "/tmp/test_litl_write.trace";
#endif

  printf("Recording events with various number of arguments\n\n");

  trace = litl_write_init_trace(buffer_size);
  litl_write_set_filename(trace, filename);

#ifdef LITL_TESTBUFFER_FLUSH
  litl_write_buffer_flush_on(trace);
#else
  litl_write_buffer_flush_off(trace);
#endif

  nb_iter = 1000;
  litl_data_t val[] =
    "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
  for (i = 0; i < (nb_iter + 1) / 12; i++) {
    litl_write_probe_reg_0(trace, 0x100 * (i + 1) + 1);
    usleep(100);
    litl_write_probe_reg_1(trace, 0x100 * (i + 1) + 2, 1);
    usleep(100);
    litl_write_probe_reg_2(trace, 0x100 * (i + 1) + 3, 1, 3);
    usleep(100);
    litl_write_probe_reg_3(trace, 0x100 * (i + 1) + 4, 1, 3, 5);
    usleep(100);
    litl_write_probe_reg_4(trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
    usleep(100);
    litl_write_probe_reg_5(trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
    usleep(100);
    litl_write_probe_reg_6(trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
    usleep(100);
    litl_write_probe_reg_7(trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13, 17);
    usleep(100);
    litl_write_probe_reg_8(trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13, 17,
                           19);
    usleep(100);
    litl_write_probe_reg_9(trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13, 17,
                           19, 23);
    usleep(100);
    litl_write_probe_reg_10(trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13, 17,
                            19, 23, 29);
    usleep(100);
    litl_write_probe_raw(trace, 0x100 * (i + 1) + 12, sizeof(val), val);
    usleep(100);
  }

  litl_write_finalize_trace(trace);

  printf("Events are recorded and written in the %s file\n", filename);

  return EXIT_SUCCESS;
}
