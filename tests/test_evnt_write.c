/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
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
    const uint32_t buffer_size = 512 * 1024; // 512KB

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    printf("=============================================================\n");
    printf("Recording events with various number of arguments\n\n");

    evnt_set_filename(filename);
    evnt_init_trace(buffer_size);

    printf("Enter the number of events to be recorded (nb >= 1): ");
    i = scanf("%d", &nb_iter);

    if (nb_iter < 12)
        nb_iter = 12;

    evnt_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < (nb_iter + 1) / 12; i++) {
        evnt_probe0(10 * i + 1);
        usleep(100);
        evnt_probe1(10 * i + 2, 1);
        usleep(100);
        evnt_probe2(10 * i + 3, 1, 3);
        usleep(100);
        evnt_probe3(10 * i + 4, 1, 3, 5);
        usleep(100);
        evnt_probe4(10 * i + 5, 1, 3, 5, 7);
        usleep(100);
        evnt_probe5(10 * i + 6, 1, 3, 5, 7, 11);
        usleep(100);
        evnt_probe6(10 * i + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        evnt_probe7(10 * i + 8, 1, 3, 5, 7, 11, 13, 17);
        usleep(100);
        evnt_probe8(10 * i + 9, 1, 3, 5, 7, 11, 13, 17, 19);
        usleep(100);
        evnt_probe9(10 * i + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
        usleep(100);
        evnt_probe10(10 * i + 11, 1, 3, 5, 7, 11, 13, 17, 19, 23, 29);
        usleep(100);
        evnt_raw_probe(10 * i + 12, sizeof(val) - 1, val);
        usleep(100);
    }

    evnt_fin_trace();

    printf("\nEvents are recorded and written in the %s file\n", filename);
    printf("=============================================================\n");

    return EXIT_SUCCESS;
}
