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

    evnt_trace_t trace;
    const char* filename = "trace";
    const uint32_t buffer_size = 512 * 1024; // 512KB

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_pause.trace";

    printf("=============================================================\n");
    printf("Recording events with various number of arguments\n\n");

    trace = evnt_init_trace(buffer_size);
    evnt_set_filename(&trace, filename);

    nb_iter = 10;

    evnt_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < nb_iter; i++) {
        if (i == nb_iter / 2) {
            printf("Loop %d: stop recording\n", i);
            evnt_pause_recording(&trace);
        }

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

        if (i == nb_iter / 2) {
            printf("Loop %d: resume recording\n", i);
            evnt_resume_recording(&trace);
        }
    }

    printf("\nEvents are recorded and written in the %s file\n", filename);
    printf("=============================================================\n");
    printf("Please check manually that %s does not contain events with code ", trace.evnt_filename);
    printf("between %x and %x\n", 100 * (nb_iter / 2) + 1, 100 * (nb_iter / 2) + 12);

    evnt_fin_trace(&trace);

    return EXIT_SUCCESS;
}
