/*
 * test_evnt_write.c
 *
 *  Created on: Nov 5, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "evnt_types.h"
#include "evnt_write.h"

int main(int argc, const char * argv[]) {
    int i, nb_iter;

    char* filename = "test_evnt_write.trace";
    buffer_flags buffer_flush = EVNT_BUFFER_FLUSH;
    thread_flags thread_safe = EVNT_NOTHREAD_SAFE;
    uint32_t buffer_size = 32 * 1024; // 32 KB

    printf("=============================================================\n");
    printf("Recording events with zero arguments\n\n");

    trace_init(filename, buffer_flush, thread_safe, buffer_size);

    printf("Enter the number of events to be recorded (nb >= 1): ");
    i = scanf("%d", &nb_iter);

    for (i = 0; i < nb_iter; i++) {
        evnt_probe0((uint64_t) 10 * i + 3);
        usleep(100);
    }

    trace_fin();

    printf("\nEvents are recorded into the %s file\n", filename);
    printf("=============================================================\n");

    return EXIT_SUCCESS;
}
