/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test is to simulate the situation when some threads start later than
 * the others while recording events of multi-threaded applications
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "litl_types.h"
#include "litl_write.h"
#include "litl_read.h"

#define NBTHREAD 16
#define NBITER 100
#define NBEVENT (NBITER * 12)

static litl_write_trace_t* __trace;

/*
 * Records several traces at the same time
 */
void* write_trace(void *arg __attribute__ ((__unused__))) {
  int i;

  litl_data_t val[] =
      "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
  for (i = 0; i < NBITER; i++) {
    litl_write_probe_reg_0(__trace, 0x100 * (i + 1) + 1);
    usleep(100);
    litl_write_probe_reg_1(__trace, 0x100 * (i + 1) + 2, 1);
    usleep(100);
    litl_write_probe_reg_2(__trace, 0x100 * (i + 1) + 3, 1, 3);
    usleep(100);
    litl_write_probe_reg_3(__trace, 0x100 * (i + 1) + 4, 1, 3, 5);
    usleep(100);
    litl_write_probe_reg_4(__trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
    usleep(100);
    litl_write_probe_reg_5(__trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
    usleep(100);
    litl_write_probe_reg_6(__trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
    usleep(100);
    litl_write_probe_reg_7(__trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13,
        17);
    usleep(100);
    litl_write_probe_reg_8(__trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13, 17,
        19);
    usleep(100);
    litl_write_probe_reg_9(__trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13,
        17, 19, 23);
    usleep(100);
    litl_write_probe_reg_10(__trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13,
        17, 19, 23, 29);
    usleep(100);
    litl_write_probe_raw(__trace, 0x100 * (i + 1) + 12, sizeof(val), val);
    usleep(100);
  }

  return NULL ;
}

void read_trace(char* filename) {
  int nb_events = 0;

  litl_read_event_t* event;
  litl_read_trace_t *trace;

  trace = litl_read_open_trace(filename);

  litl_read_init_processes(trace);

  while (1) {
    event = litl_read_next_event(trace);

    if (event == NULL )
      break;

    nb_events++;
  }

  litl_read_finalize_trace(trace);

  if (nb_events != NBEVENT * NBTHREAD) {
    fprintf(stderr,
        "Some events were NOT recorded!\n Expected nb_events = %d \t Recorded nb_events = %d\n",
        NBEVENT * NBTHREAD, nb_events);
    exit(EXIT_FAILURE);
  }
}

int main() {
  int i, res __attribute__ ((__unused__));
  char* filename;
  pthread_t tid[NBTHREAD];
  const uint32_t buffer_size = 1024; // 1KB

  printf("Recording events by %d threads\n\n", NBTHREAD);
  res = asprintf(&filename, "/tmp/test_litl_write_multiple_threads.trace");

  __trace = litl_write_init_trace(buffer_size);
  litl_write_set_filename(__trace, filename);

  for (i = 0; i < NBTHREAD; i++) {
    pthread_create(&tid[i], NULL, write_trace, &i);
    if ((i + 1) % 4 == 0) {
      sleep(1);
    }
  }

  for (i = 0; i < NBTHREAD; i++)
    pthread_join(tid[i], NULL );

  printf("All events are stored in %s\n\n", __trace->filename);
  litl_write_finalize_trace(__trace);

  printf("Checking the recording of events\n\n");

  read_trace(filename);

  printf("Yes, the events were recorded successfully\n");

  return EXIT_SUCCESS;
}
