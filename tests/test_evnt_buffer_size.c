/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "evnt_types.h"
#include "evnt_write.h"
#include "evnt_read.h"
#include "timer.h"

#define MAX_BUFFER_SIZE 16 * 1024 * 1024
#define NB_EVENTS 1000000

int main(int argc, const char **argv) {
    int i;
    const char* filename = "trace.trace";
    uint32_t buf_size;

    evnt_time_t start, fin;
    evnt_trace_t trace;
    evnt_t* event;
    evnt_buffer_t buffer;
    evnt_block_t block;
    evnt_info_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    printf("=============================================================\n");
    printf("What is the optimal buffer size for recording events?\n\n");
    printf("Buffer size[KB] \t Time \n");

    evnt_data_t val[] =
            "Well, that's Philosophy I've read, And Law and Medicine, and I fear Theology, too, from A to Z; Hard studies all, that have cost me dear. And so I sit, poor silly man No wiser now than when I began.";
    buf_size = 1024; // 1KB
    while (buf_size <= MAX_BUFFER_SIZE) {
        trace = evnt_init_trace(buf_size);
        evnt_set_filename(&trace, filename);

        start = evnt_get_time();
        for (i = 0; i < (NB_EVENTS + 1) / 12; i++) {
            evnt_probe0(&trace, 10 * i + 1);
            evnt_probe1(&trace, 10 * i + 2, 1);
            evnt_probe2(&trace, 10 * i + 3, 1, 3);
            evnt_probe3(&trace, 10 * i + 4, 1, 3, 5);
            evnt_probe4(&trace, 10 * i + 5, 1, 3, 5, 7);
            evnt_probe5(&trace, 10 * i + 6, 1, 3, 5, 7, 11);
            evnt_probe6(&trace, 10 * i + 7, 1, 3, 5, 7, 11, 13);
            evnt_probe7(&trace, 10 * i + 8, 1, 3, 5, 7, 11, 13, 17);
            evnt_probe8(&trace, 10 * i + 9, 1, 3, 5, 7, 11, 13, 17, 19);
            evnt_probe9(&trace, 10 * i + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
            evnt_probe10(&trace, 10 * i + 11, 1, 3, 5, 7, 11, 13, 17, 19, 23, 29);
            evnt_raw_probe(&trace, 10 * i + 12, sizeof(val) - 1, val);
        }
        fin = evnt_get_time();

        evnt_fin_trace(&trace);
        printf("\t%"PRTIu32"\t\t %"PRTIu64"\n", buf_size / 1024, fin - start);

        buf_size = 2 * buf_size;
    }
    printf("=============================================================\n");
    printf("NB: time was measured only once on writing %d events to the %s file.\n\n", NB_EVENTS, filename);

    printf("\n");
    printf("=============================================================\n");
    printf("What is the optimal buffer size for reading the trace file?\n\n");
    printf("Buffer size[KB] \t Time \n");

    buf_size = 1024; // 1KB
    while (buf_size <= MAX_BUFFER_SIZE) {
        set_read_buffer_size(buf_size);
        buffer = evnt_open_trace(filename);
        block = evnt_get_block(buffer);
        //        header = evnt_get_trace_header(&block);

        start = evnt_get_time();
        while (block.buffer != NULL ) {
            event = evnt_read_event(&block);

            if (event == NULL )
                break;
        }
        fin = evnt_get_time();

        evnt_close_trace(&block);
        printf("\t%"PRTIu32"\t\t %"PRTIu64"\n", buf_size / 1024, fin - start);

        buf_size = 2 * buf_size;
    }
    printf("=============================================================\n");
    printf("NB: time was measured only once on reading %d events from the %s file.\n\n", NB_EVENTS, filename);

    return EXIT_SUCCESS;
}
