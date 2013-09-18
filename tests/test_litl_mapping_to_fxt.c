/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test validates the mapping between LiTL and FxT APIs
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "fxt.h"
#include "fut.h"

#define THREAD_ID pthread_self()

int main() {
  int i, nb_iter;

  char* filename = "/tmp/test_litl_fxt_trace_size.trace";
  const uint32_t buffer_size = 16 * 1024 * 1024; // 16MB
  nb_iter = 1000;

  fut_setup(buffer_size, FUT_KEYMASKALL, THREAD_ID);

  fut_set_filename(filename);
  enable_fut_flush()
  ;
  fut_enable_tid_logging()
  ;

  for (i = 0; i < nb_iter; i++) {
    // char events
    FUT_DO_PROBE0(0x100 * (i + 1) + 0);
    FUT_DO_PROBE1(0x100 * (i + 1) + 1, (uint8_t ) 1);
    FUT_DO_PROBE2(0x100 * (i + 1) + 2, (uint8_t ) 1, (uint8_t ) 3);
    FUT_DO_PROBE3(0x100 * (i + 1) + 3, (uint8_t ) 1, (uint8_t ) 3,
        (uint8_t ) 5);
    FUT_DO_PROBE4(0x100 * (i + 1) + 4, (uint8_t ) 1, (uint8_t ) 3, (uint8_t ) 5,
        (uint8_t ) 7);
    FUT_DO_PROBE5(0x100 * (i + 1) + 5, (uint8_t ) 1, (uint8_t ) 3, (uint8_t ) 5,
        (uint8_t ) 7, (uint8_t ) 11);
    FUT_DO_PROBE6(0x100 * (i + 1) + 6, (uint8_t ) 1, (uint8_t ) 3, (uint8_t ) 5,
        (uint8_t ) 7, (uint8_t ) 11, (uint8_t ) 13);
    usleep(10);

    // int events
    FUT_DO_PROBE0(0x100 * (i + 1) + 7);
    FUT_DO_PROBE1(0x100 * (i + 1) + 8, (int ) 1);
    FUT_DO_PROBE2(0x100 * (i + 1) + 9, (int ) 1, (int ) 3);
    FUT_DO_PROBE3(0x100 * (i + 1) + 10, (int ) 1, (int ) 3, (int ) 5);
    FUT_DO_PROBE4(0x100 * (i + 1) + 11, (int ) 1, (int ) 3, (int ) 5, (int ) 7);
    FUT_DO_PROBE5(0x100 * (i + 1) + 12, (int ) 1, (int ) 3, (int ) 5, (int ) 7,
        (int ) 11);
    FUT_DO_PROBE6(0x100 * (i + 1) + 13, (int ) 1, (int ) 3, (int ) 5, (int ) 7,
        (int ) 11, (int ) 13);
    usleep(10);

    // double events
    FUT_DO_PROBE0(0x100 * (i + 1) + 14);
    FUT_DO_PROBE1(0x100 * (i + 1) + 15, (double ) 1);
    FUT_DO_PROBE2(0x100 * (i + 1) + 16, (double ) 1, (double ) 3);
    FUT_DO_PROBE3(0x100 * (i + 1) + 17, (double ) 1, (double ) 3, (double ) 5);
    FUT_DO_PROBE4(0x100 * (i + 1) + 18, (double ) 1, (double ) 3, (double ) 5,
        (double ) 7);
    FUT_DO_PROBE5(0x100 * (i + 1) + 19, (double ) 1, (double ) 3, (double ) 5,
        (double ) 7, (double ) 11);
    FUT_DO_PROBE6(0x100 * (i + 1) + 20, (double ) 1, (double ) 3, (double ) 5,
        (double ) 7, (double ) 11, (double ) 13);
    usleep(10);
  }

  fut_endup(filename);
  fut_done();

  uint64_t size;
  FILE* fp = fopen(filename, "r");
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fclose(fp);

  printf("Trace file size = %ld KB\n", size / 1024); // size in KB

  return EXIT_SUCCESS;
}
