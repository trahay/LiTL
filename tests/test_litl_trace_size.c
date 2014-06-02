/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test is to show that LiTL uses the optimized event storage and occupies
 * the required space only.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

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
    filename = "/tmp/test_litl_trace_size.trace";

  printf("Recording events with six arguments of type uint8_t\n\n");

  trace = litl_write_init_trace(buffer_size);
  litl_write_set_filename(trace, filename);
  litl_write_buffer_flush_on(trace);

  nb_iter = 1000;
  for (i = 0; i < nb_iter; i++) {
    litl_t *retval;
    litl_write_probe_pack_6(trace, 0x100 * (i + 1) + 6, (int32_t ) 1,
                            (int32_t ) 3, (int32_t ) 5, (int32_t ) 7,
                            (int32_t ) 11, (int32_t ) 13, retval);
    assert(retval != NULL);
    usleep(100);
  }

  litl_write_finalize_trace(trace);

  printf("Events are recorded and written in the %s file\n", filename);

  litl_param_t size;
  FILE* fp = fopen(filename, "r");
  fseek(fp, 0L, SEEK_END);
  size = ftell(fp);
  fclose(fp);

  printf("\nThe size of the trace file with %d packed event6 is %ld bytes \n",
         nb_iter, size);

  return EXIT_SUCCESS;
}
