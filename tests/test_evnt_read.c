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
    read_evnt_t* event;
    evnt_trace_read_t *trace;
    evnt_header_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_evnt_write.trace";

    trace = evnt_open_trace(filename);
    header = evnt_get_trace_header(trace);

    // print the header
    printf(" libevnt v.%s\n", header->libevnt_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    printf("Thread ID\t Event Type & Code \t Time\t   NB args\t Arguments[0-9]\n");
    while (1) {
        event = evnt_next_buffer_event(trace, index);

        if (event == NULL )
            break;

        switch (EVENT_GET_TYPE(event)) {
        case EVNT_TYPE_REGULAR: { // regular event
	  printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, EVENT_GET_TID(event), EVENT_GET_CODE(event),
		 EVENT_GET_TIME(event), EVENT_REGULAR(event)->nb_params);

            for (i = 0; i < EVENT_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, EVENT_REGULAR(event)->param[i]);
            break;
        }
        case EVNT_TYPE_RAW: { // raw event
	    EVENT_GET_CODE(event) = clear_bit(EVENT_GET_CODE(event));
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, EVENT_GET_TID(event), EVENT_GET_CODE(event),
		   EVENT_GET_TIME(event), EVENT_RAW(event)->size);
            printf("\t %s", (evnt_data_t *) EVENT_RAW(event)->data);
            break;
        }
        case EVNT_TYPE_PACKED: { // packed event
	  printf("%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, EVENT_GET_TID(event),
		 EVENT_GET_CODE(event), EVENT_GET_TIME(event), EVENT_PACKED(event)->size);
            for (i = 0; i < EVENT_PACKED(event)->size; i++) {
                printf(" %x", EVENT_PACKED(event)->param[i]);
            }
            break;
        }
        case EVNT_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", EVENT_GET_TYPE(event));
            *(int*) 0 = 0;
        }
        }

        printf("\n");
    }

    evnt_close_trace(trace);

    return EXIT_SUCCESS;
}
