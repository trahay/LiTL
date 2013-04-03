/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#include "evnt_types.h"
#include "evnt_write.h"

#define NBTHREAD 4

/*
 * Only used during thread creating to make sure that the thread
 * got the correct args.
 */
sem_t thread_ready;

/*
 * This test records several traces at the same time
 */
void* write_trace(void* arg) {
    int i;
    char* filename;
    uint8_t my_id = *(uint8_t*) arg;

    // Notify the main thread that we got the args
    sem_post(&thread_ready);

    printf("Recording events on thread #%d\n", my_id);
    asprintf(&filename, "/tmp/test_evnt_write_%d.trace", my_id);

    evnt_trace_t trace;
    const uint32_t buffer_size = 512 * 1024; // 512KB

    trace = evnt_init_trace(buffer_size);
    evnt_set_filename(&trace, filename);

    int nb_iter = 1000;
    evnt_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < nb_iter; i++) {
        evnt_probe0(&trace, 0x100 * i + 1);
        usleep(100);
        evnt_probe1(&trace, 0x100 * i + 2, 1);
        usleep(100);
        evnt_probe2(&trace, 0x100 * i + 3, 1, 3);
        usleep(100);
        evnt_probe3(&trace, 0x100 * i + 4, 1, 3, 5);
        usleep(100);
        evnt_probe4(&trace, 0x100 * i + 5, 1, 3, 5, 7);
        usleep(100);
        evnt_probe5(&trace, 0x100 * i + 6, 1, 3, 5, 7, 11);
        usleep(100);
        evnt_probe6(&trace, 0x100 * i + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        evnt_probe7(&trace, 0x100 * i + 8, 1, 3, 5, 7, 11, 13, 17);
        usleep(100);
        evnt_probe8(&trace, 0x100 * i + 9, 1, 3, 5, 7, 11, 13, 17, 19);
        usleep(100);
        evnt_probe9(&trace, 0x100 * i + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
        usleep(100);
        evnt_probe10(&trace, 0x100 * i + 11, 1, 3, 5, 7, 11, 13, 17, 19, 23, 29);
        usleep(100);
        evnt_raw_probe(&trace, 0x100 * i + 12, sizeof(val) - 1, val);
        usleep(100);
    }

    printf("Events for thread #%d are stored in %s\n", my_id, trace.evnt_filename);

    evnt_fin_trace(&trace);

    return NULL;
}

int main(int argc, const char **argv) {
    int i;
    pthread_t tid[NBTHREAD];

    sem_init(&thread_ready, 0, 0);

    for (i = 0; i < NBTHREAD; i++) {
        pthread_create(&tid[i], NULL, write_trace, &i);
        sem_wait(&thread_ready);
    }

    for (i = 0; i < NBTHREAD; i++)
        pthread_join(tid[i], NULL );

    return EXIT_SUCCESS;
}
