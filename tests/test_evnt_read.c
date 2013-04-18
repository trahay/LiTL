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
    evnt_size_t i, index;
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

    printf("Thread ID\t Event Type & Code \t Time\t   NB args\t Arguments[0-9]\n");
    while (1) {
        event = evnt_next_buffer_event(&trace, index);

        if (event == NULL )
            break;

        switch (event->type) {
        case EVNT_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, trace.tids[index]->tid, event->code,
                    event->time, event->parameters.regular.nb_params);

            for (i = 0; i < event->parameters.regular.nb_params; i++)
                printf("\t %"PRTIx64, event->parameters.regular.param[i]);
            break;
        }
        case EVNT_TYPE_RAW: { // raw event
            event->code = clear_bit(event->code);
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, trace.tids[index]->tid, event->code,
                    event->time, event->parameters.raw.size);
            printf("\t %s", (evnt_data_t *) event->parameters.raw.data);
            break;
        }
        case EVNT_TYPE_PACKED: { // packed event
            printf("%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, trace.tids[index]->tid,
                    event->code, event->time, event->parameters.packed.size);
            for (i = 0; i < event->parameters.packed.size; i++) {
                printf("\t%x", event->parameters.packed.param[i]);
            }
            break;
        }
        case EVNT_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", event->type);
            *(int*) 0 = 0;
        }
        }

        printf("\n");
    }

    evnt_close_trace(&trace);

    return EXIT_SUCCESS;
}
