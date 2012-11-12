/*
 * test_evnt_buffer_size.c
 *
 *  Created on: Nov 12, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "evnt_types.h"
#include "evnt_read.h"
#include "timer.h"

#define MAX_BUFFER_SIZE 16 * 1024 * 1024

int main(int argc, const char **argv) {
    long long int start, fin;
    uint64_t j;
    const char* filename = "trace";
    evnt* event;
    trace buffer;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        fprintf(stderr, "Specify the name of the trace file using '-f'\n");
        exit(-1);
    }

    printf("=============================================================\n");
    printf("What is the optimal buffer size for reading the trace file?\n\n");
    printf("Buffer size (KB) \t Time \n");

    j = 1024;
    while (j <= MAX_BUFFER_SIZE) {
        set_buffer_size(j);
        buffer = open_trace(filename);

        start = get_ticks();
        while (buffer != NULL ) {
            event = read_event(&buffer);

            if (event == NULL )
                break;
        }
        fin = get_ticks();

        printf("\t%lu\t\t %llu\n", j / 1024, fin - start);
        close_trace(&buffer);

        j = 2 * j;
    }
    printf("=============================================================\n");
    printf("NB: time was measured only once on ready all events from the %s trace file.\n\n", filename);

    return EXIT_SUCCESS;
}
