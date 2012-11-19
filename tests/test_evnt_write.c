/*
 * test_evnt_write.c
 *
 *  Created on: Nov 5, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "evnt_types.h"
#include "evnt_write.h"

int main(int argc, const char **argv) {
    int i, nb_iter;

    const char* filename = "trace";
    uint32_t buffer_size = 256 * 1024; // 256 KB

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    printf("=============================================================\n");
    printf("Recording events with zero arguments\n\n");

    init_trace(filename, buffer_size);

    printf("Enter the number of events to be recorded (nb >= 1): ");
    i = scanf("%d", &nb_iter);

    if (nb_iter < 9)
        nb_iter = 9;

    for (i = 0; i < (nb_iter + 1) / 10; i++) {
        evnt_probe0((uint64_t) 10 * i + 1);
        usleep(100);
        evnt_probe1((uint64_t) 10 * i + 2, 1);
        usleep(100);
        evnt_probe2((uint64_t) 10 * i + 3, 1, 3);
        usleep(100);
        evnt_probe3((uint64_t) 10 * i + 4, 1, 3, 5);
        usleep(100);
        evnt_probe4((uint64_t) 10 * i + 5, 1, 3, 5, 7);
        usleep(100);
        evnt_probe5((uint64_t) 10 * i + 6, 1, 3, 5, 7, 11);
        usleep(100);
        evnt_probe6((uint64_t) 10 * i + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        evnt_probe7((uint64_t) 10 * i + 8, 1, 3, 5, 7, 11, 13, 17);
        usleep(100);
        evnt_probe8((uint64_t) 10 * i + 9, 1, 3, 5, 7, 11, 13, 17, 19);
        usleep(100);
        evnt_probe9((uint64_t) 10 * i + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
        usleep(100);
    }

    fin_trace();

    printf("\nEvents are recorded and written in the %s file\n", filename);
    printf("=============================================================\n");

    return EXIT_SUCCESS;
}
