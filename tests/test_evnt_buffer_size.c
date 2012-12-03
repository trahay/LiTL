/*
 * test_evnt_buffer_size.c
 *
 *  Created on: Nov 12, 2012
 *      Author: Roman Iakymchuk
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
    long long int start, fin;
    const char* filename = "trace";
    uint32_t buf_size;
    evnt_t* event;
    trace_t buffer;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        fprintf(stderr, "Specify the name of the trace file using '-f'\n");
        exit(-1);
    }

    printf("=============================================================\n");
    printf("What is the optimal buffer size for recording events?\n\n");
    printf("Buffer size (KB) \t Time \n");

    buf_size = 1024; // 1KB
    while (buf_size <= MAX_BUFFER_SIZE) {
        init_trace(filename, buf_size);

        start = get_ticks();
        for (i = 0; i < (NB_EVENTS + 1) / 10; i++) {
            evnt_probe0(10 * i + 1);
            evnt_probe1(10 * i + 2, 1);
            evnt_probe2(10 * i + 3, 1, 3);
            evnt_probe3(10 * i + 4, 1, 3, 5);
            evnt_probe4(10 * i + 5, 1, 3, 5, 7);
            evnt_probe5(10 * i + 6, 1, 3, 5, 7, 11);
            evnt_probe6(10 * i + 7, 1, 3, 5, 7, 11, 13);
            evnt_probe7(10 * i + 8, 1, 3, 5, 7, 11, 13, 17);
            evnt_probe8(10 * i + 9, 1, 3, 5, 7, 11, 13, 17, 19);
            evnt_probe9(10 * i + 10, 1, 3, 5, 7, 11, 13, 17, 19, 23);
        }
        fin = get_ticks();

        fin_trace();
        printf("\t%u\t\t %llu\n", buf_size / 1024, fin - start);

        buf_size = 2 * buf_size;
    }
    printf("=============================================================\n");
    printf("NB: time was measured only once on writing %d events to the %s file.\n\n", NB_EVENTS, filename);

    printf("\n");
    printf("=============================================================\n");
    printf("What is the optimal buffer size for reading the trace file?\n\n");
    printf("Buffer size (KB) \t Time \n");

    buf_size = 1024; // 1KB
    while (buf_size <= MAX_BUFFER_SIZE) {
        set_read_buffer_size(buf_size);
        buffer = open_trace(filename);

        start = get_ticks();
        while (buffer != NULL ) {
            event = read_event(&buffer);

            if (event == NULL )
                break;
        }
        fin = get_ticks();

        close_trace(&buffer);
        printf("\t%u\t\t %llu\n", buf_size / 1024, fin - start);

        buf_size = 2 * buf_size;
    }
    printf("=============================================================\n");
    printf("NB: time was measured only once on reading %d events from the %s file.\n\n", NB_EVENTS, filename);

    return EXIT_SUCCESS;
}
