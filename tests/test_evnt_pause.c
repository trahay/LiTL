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
#include "evnt_read.h"

void write_trace(char* filename, int nb_iter, int skipped_iter) {
    int i;

    evnt_trace_t trace;
    const uint32_t buffer_size = 512 * 1024; // 512KB

    trace = evnt_init_trace(buffer_size);
    evnt_set_filename(&trace, filename);

    evnt_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    for (i = 0; i < nb_iter; i++) {
        if (i == skipped_iter) {
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

        if (i == skipped_iter) {
            printf("Loop %d: resume recording\n", i);
            evnt_resume_recording(&trace);
        }
    }

    printf("Events with code between %x and %x were not recorded\n", 0x100 * skipped_iter + 1, 0x100 * skipped_iter + 12);

    evnt_fin_trace(&trace);
}

void read_trace(char* filename, int left_bound, int right_bound) {
    int nbevents = 0;

    const evnt_size_t buffer_size = 16 * 1024 * 1024; // 16MB
    evnt_t* event;
    evnt_buffer_t buffer;
    evnt_block_t block;

    set_read_buffer_size(buffer_size);
    buffer = evnt_open_trace(filename);
    block = evnt_get_block(buffer);

    while (block.buffer != NULL ) {
        event = evnt_read_event(&block);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 1)
            // raw event
            event->code = clear_bit(event->code);

        // check whether some events were skipped
        if ((left_bound < event->code) && (event->code < right_bound)) {
            nbevents++;
            break;
        }
    }

    evnt_close_trace(&block);

    if (nbevents > 0) {
        fprintf(stderr, "Some events were recorded when they supposed to be skipped");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char **argv) {
    int nb_iter;
    int skipped_iter;
    const char* filename = "trace";

    nb_iter = 10;
    skipped_iter = nb_iter / 2;
    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_evnt_pause.trace";

    printf("=============================================================\n");
    printf("Recording events with various number of arguments\n\n");

    write_trace(filename, nb_iter, skipped_iter);

    printf("\nEvents are recorded and written in the %s file\n", filename);

    printf("\nChecking whether the recording of events was paused\n");

    read_trace(filename, 0x100 * skipped_iter + 1, 0x100 * skipped_iter + 12);

    printf("Yes, the recording of events was paused\n");

    printf("=============================================================\n");

    return EXIT_SUCCESS;
}
