/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test simulates recording of events by multiple threads simultaneously.
 * This test ensures that LiTL can be used by many applications at the same time
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#include "litl_types.h"
#include "litl_write.h"

#define NBTHREAD 4

/*
 * Only used during thread creating to make sure that the thread
 * got the correct args.
 */
sem_t thread_ready;

/*
 * Records several traces at the same time
 */
void* write_trace(void* arg) {
    int i, res __attribute__ ((__unused__));
    char* filename;
    uint8_t my_id = *(uint8_t*) arg;

    // Notify the main thread that we got the args
    sem_post(&thread_ready);

    printf("Recording events on thread #%d\n", my_id);
    res = asprintf(&filename, "/tmp/test_litl_write_%d.trace", my_id);

    litl_trace_write_t trace;
    const uint32_t buffer_size = 512 * 1024; // 512KB

    trace = litl_init_trace(buffer_size);
    litl_set_filename(&trace, filename);

    int nb_iter = 1000;
    litl_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < nb_iter; i++) {
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
        litl_probe_raw(&trace, 0x100 * (i + 1) + 12, sizeof(val) - 1, val);
        usleep(100);
    }

    printf("Events for thread #%d are stored in %s\n", my_id, trace.filename);

    litl_fin_trace(&trace);

    return NULL ;
}

int main() {
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
