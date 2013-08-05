/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test simulates recording of events of multi-threaded applications.
 * The test ensures that LiTL solves the race conditions by recording events
 * into separate buffers: one per thread.
 * All the buffers are written to the same trace file
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

litl_trace_write_t trace;

/*
 * This test records several traces at the same time
 */
void* write_trace(void *arg) {
    int i;

    litl_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < NBITER; i++) {
        litl_probe_reg_0(&trace, 0x100 * (i + 1) + 1);
        usleep(100);
        litl_probe_reg_1(&trace, 0x100 * (i + 1) + 2, 1);
        usleep(100);
        litl_probe_reg_2(&trace, 0x100 * (i + 1) + 3, 1, 3);
        usleep(100);
        litl_probe_reg_3(&trace, 0x100 * (i + 1) + 4, 1, 3, 5);
        usleep(100);
        litl_probe_reg_4(&trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
        usleep(100);
        litl_probe_reg_5(&trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
        usleep(100);
        litl_probe_reg_6(&trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        litl_probe_reg_7(&trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13, 17);
        usleep(100);
        litl_probe_reg_8(&trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13, 17,
                19);
        usleep(100);
        litl_probe_reg_9(&trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13, 17,
                19, 23);
        usleep(100);
        litl_probe_reg_10(&trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13, 17,
                19, 23, 29);
        usleep(100);
        litl_probe_raw(&trace, 0x100 * (i + 1) + 12, sizeof(val), val);
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
        fprintf(stderr,
                "Some events were NOT recorded!\n Expected nb_events = %d \t Recorded nb_events = %d\n",
                NBEVENT * NBTHREAD, nb_events);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int i;
    char* filename;
    pthread_t tid[NBTHREAD];
    const uint32_t buffer_size = 1024; // 1KB

    printf("Recording events by %d threads\n\n", NBTHREAD);
    asprintf(&filename, "/tmp/test_litl_write_concurent.trace");

    trace = litl_init_trace(buffer_size);
    litl_set_filename(&trace, filename);

    for (i = 0; i < NBTHREAD; i++) {
        pthread_create(&tid[i], NULL, write_trace, &i);
    }

    for (i = 0; i < NBTHREAD; i++)
        pthread_join(tid[i], NULL );

    printf("All events are stored in %s\n\n", trace.filename);
    litl_fin_trace(&trace);

    printf("Checking the recording of events\n\n");

    read_trace(filename);

    printf("Yes, the events were recorded successfully\n");

    return EXIT_SUCCESS;
}
