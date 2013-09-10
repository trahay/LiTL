/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test simulates the situation when the event recoding is paused.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "litl_types.h"
#include "litl_tools.h"
#include "litl_write.h"
#include "litl_read.h"

void write_trace(char* filename, int nb_iter, int skipped_iter) {
    int i;

    litl_write_trace_t* trace;
    const uint32_t buffer_size = 512 * 1024; // 512KB

    trace = litl_write_init_trace(buffer_size);
    litl_write_set_filename(trace, filename);

    litl_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < nb_iter; i++) {
        if (i == skipped_iter - 1) {
            printf("Loop %d: stop recording\n", i);
            litl_write_pause_recording(trace);
        }

        litl_write_probe_reg_0(trace, 0x100 * (i + 1) + 1);
        usleep(100);
        litl_write_probe_reg_1(trace, 0x100 * (i + 1) + 2, 1);
        usleep(100);
        litl_write_probe_reg_2(trace, 0x100 * (i + 1) + 3, 1, 3);
        usleep(100);
        litl_write_probe_reg_3(trace, 0x100 * (i + 1) + 4, 1, 3, 5);
        usleep(100);
        litl_write_probe_reg_4(trace, 0x100 * (i + 1) + 5, 1, 3, 5, 7);
        usleep(100);
        litl_write_probe_reg_5(trace, 0x100 * (i + 1) + 6, 1, 3, 5, 7, 11);
        usleep(100);
        litl_write_probe_reg_6(trace, 0x100 * (i + 1) + 7, 1, 3, 5, 7, 11, 13);
        usleep(100);
        litl_write_probe_reg_7(trace, 0x100 * (i + 1) + 8, 1, 3, 5, 7, 11, 13,
                17);
        usleep(100);
        litl_write_probe_reg_8(trace, 0x100 * (i + 1) + 9, 1, 3, 5, 7, 11, 13,
                17, 19);
        usleep(100);
        litl_write_probe_reg_9(trace, 0x100 * (i + 1) + 10, 1, 3, 5, 7, 11, 13,
                17, 19, 23);
        usleep(100);
        litl_write_probe_reg_10(trace, 0x100 * (i + 1) + 11, 1, 3, 5, 7, 11, 13,
                17, 19, 23, 29);
        usleep(100);
        litl_write_probe_raw(trace, 0x100 * (i + 1) + 12, sizeof(val) - 1, val);
        usleep(100);

        if (i == skipped_iter - 1) {
            printf("Loop %d: resume recording\n", i);
            litl_write_resume_recording(trace);
        }
    }

    printf("\nEvents with code between %x and %x were not recorded\n",
            0x100 * skipped_iter + 1, 0x100 * skipped_iter + 12);

    litl_write_finalize_trace(trace);
}

void read_trace(char* filename, uint32_t left_bound, uint32_t right_bound) {
    int nbevents = 0;

    litl_read_event_t* event;
    litl_read_trace_t *arch;

    arch = litl_read_open_trace(filename);

    litl_read_init_processes(arch);

    while (1) {
        event = litl_read_next_event(arch);

        if (event == NULL )
            break;

        // check whether some events were skipped
        if ((left_bound < LITL_READ_GET_CODE(event))&& (LITL_READ_GET_CODE(event) < right_bound)){
        nbevents++;
        break;
    }
}

    litl_read_finalize_trace(arch);

    if (nbevents > 0) {
        fprintf(stderr,
                "Some events were recorded when they supposed to be skipped");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    int nb_iter;
    int skipped_iter;
    char* filename = "trace";

    nb_iter = 10;
    skipped_iter = nb_iter / 2;
    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_litl_pause.trace";

    printf("Recording events with various number of arguments\n\n");

    write_trace(filename, nb_iter, skipped_iter);

    printf("Events are recorded and written in the %s file\n", filename);

    printf("\nChecking whether the recording of events was paused\n");

    read_trace(filename, 0x100 * skipped_iter + 1, 0x100 * skipped_iter + 12);

    printf("Yes, the recording of events was paused\n");

    return EXIT_SUCCESS;
}
