/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evnt_types.h"
#include "evnt_read.h"
#include "evnt_macro.h"

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_t* event;
    evnt_trace_read_t trace;
    evnt_header_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_evnt_write.trace";

    trace = evnt_open_trace(filename);
    header = evnt_get_trace_header(&trace);

    // print the header
    printf(" libevnt v.%s\n", header->libevnt_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    printf("Event Code \t Thread ID\t Time\t\t   NB args\t Arguments[0-9]\n");

    evnt_size_t index;
    index = 0;
    while (trace.buffer != NULL ) {
        event = evnt_next_buffer_event(&trace, index);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            if (event->code == EVNT_OFFSET) {
                // an event with code and offset
                continue;
            } else {
                // regular event
                printf("%"PRTIu64"\t %"PRTIu64"\t %"PRTIx32"\t %"PRTIu32, trace.tids[index]->tid, event->time,
                        event->code, event->nb_params);

                for (i = 0; i < event->nb_params; i++)
                    printf("\t %"PRTIx64, event->param[i]);
            }
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%"PRTIu64"\t %"PRTIu64"\t %"PRTIx32"\t %"PRTIu32, trace.tids[index]->tid, event->time, event->code,
                    event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    evnt_close_trace(&trace);

    return EXIT_SUCCESS;
}
