/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */
/*
 * The aim of this test is to simulate the situation when some threads start later than the others while recording
 *   and reading events of multi-threaded applications.
 * So, the threads information is stored by portions within the trace.
 * This test shows that LiTL solves the race conditions by recording events separately to a buffer of each thread.
 * Afterwards, the buffers are stored in one file
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

#define NBTHREAD 32
#define NBITER 100
#define NBEVENT (NBITER * 12)

static litl_trace_write_t __trace;

//static pthread_barrier_t __barrier;

/*
 * This test records several traces at the same time
 */
void* write_trace(void *arg) {
    int i;

    litl_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < NBITER; i++) {
        litl_probe0(&__trace, 0x100 * (i + 1) + 1);
        usleep(100);
        litl_probe1(&__trace, 0x100 * (i + 1) + 2, 1);
        usleep(100);
        litl_probe2(&__trace, 0x100 * (i + 1) + 3, 1, 3);
        usleep(100);
        litl_probe3(&__trace, 0x100 * (i + 1) + 4, 1, 3, 5);
        usleep(100);
        litl_probe4(&__trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
        usleep(100);
        litl_probe5(&__trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
        usleep(100);
        litl_probe6(&__trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        litl_probe7(&__trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13, 17);
        usleep(100);
        litl_probe8(&__trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13, 17, 19);
        usleep(100);
        litl_probe9(&__trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
        usleep(100);
        litl_probe10(&__trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13, 17, 19, 23, 29);
        usleep(100);
        litl_raw_probe(&__trace, 0x100 * (i + 1) + 12, sizeof(val), val);
        usleep(100);
    }

    return NULL ;
}

void read_trace(char* filename) {
    int nb_events = 0;

    litl_read_t* event;
    litl_trace_read_t *arch;

    arch = litl_open_trace(filename);

    litl_init_traces(arch);

    while (1) {
        event = litl_next_event(arch);

        if (event == NULL )
            break;

        nb_events++;
    }

    litl_close_trace(arch);

    if (nb_events != NBEVENT * NBTHREAD) {
        fprintf(stderr, "Some events were NOT recorded!\n Expected nb_events = %d \t Recorded nb_events = %d\n",
                NBEVENT * NBTHREAD, nb_events);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int i;
    char* filename;
    pthread_t tid[NBTHREAD];
    const uint32_t buffer_size = 1024; // 1KB

    //    pthread_barrier_init(&__barrier, NULL, 4);
    printf("Recording events by %d threads\n\n", NBTHREAD);
    asprintf(&filename, "/tmp/test_litl_write_multiple_threads.trace");

    __trace = litl_init_trace(buffer_size);
    litl_set_filename(&__trace, filename);

    for (i = 0; i < NBTHREAD; i++) {
        pthread_create(&tid[i], NULL, write_trace, &i);
        if ((i + 1) % 4 == 0) {
            sleep(1);
        }
    }

    for (i = 0; i < NBTHREAD; i++)
        pthread_join(tid[i], NULL );

    printf("All events are stored in %s\n\n", __trace.filename);
    litl_fin_trace(&__trace);
    //    pthread_barrier_destroy(&__barrier);

    printf("Checking the recording of events\n\n");

    read_trace(filename);

    printf("Yes, the events were recorded successfully\n");

    return EXIT_SUCCESS;
}
