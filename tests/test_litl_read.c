/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

/*
 * This test validates the event reading mechanism
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "litl_types.h"
#include "litl_tools.h"
#include "litl_read.h"

int main(int argc, const char **argv) {
    litl_data_t i;
    const char* filename = "trace";
    litl_read_event_t* event;
    litl_read_trace_t *trace;
    litl_general_header_t* trace_header;
    litl_process_header_t* process_header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_litl_write.trace";

    trace = litl_read_open_trace(filename);
    litl_read_init_processes(trace);

    trace_header = litl_read_get_trace_header(trace);
    process_header = litl_read_get_process_header(trace->processes[0]);

    // print the header
    printf(" LiTL v.%s\n", trace_header->litl_ver);
    printf(" %s\n", trace_header->sysinfo);
    printf(" nb_processes \t %d\n", trace_header->nb_processes);

    if (trace_header->nb_processes == 1)
        printf(" nb_threads \t %u\n", process_header->nb_threads);
    printf(" buffer_size \t %u\n",
            process_header->buffer_size
                    - __litl_get_reg_event_size(LITL_MAX_PARAMS)
                    - __litl_get_reg_event_size(0));

    printf(
            "Thread ID\t Event Type & Code \t Time\t   NB args\t Arguments[0-9]\n");
    while (1) {
        event = litl_read_next_event(trace);

        if (event == NULL )
            break;

        switch (LITL_READ_GET_TYPE(event)) {
        case LITL_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32,
                    LITL_READ_GET_TID(event), LITL_READ_GET_CODE(event),
                    LITL_READ_GET_TIME(event),
                    LITL_READ_REGULAR(event)->nb_params);

            for (i = 0; i < LITL_READ_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, LITL_READ_REGULAR(event)->param[i]);
            break;
        }
        case LITL_TYPE_RAW: { // raw event
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32,
                    LITL_READ_GET_TID(event), LITL_READ_GET_CODE(event),
                    LITL_READ_GET_TIME(event), LITL_READ_RAW(event)->size);
            printf("\t %s", (litl_data_t *) LITL_READ_RAW(event)->data);
            break;
        }
        case LITL_TYPE_PACKED: { // packed event
            printf(
                    "%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32,
                    LITL_READ_GET_TID(event), LITL_READ_GET_CODE(event),
                    LITL_READ_GET_TIME(event), LITL_READ_PACKED(event)->size);
            for (i = 0; i < LITL_READ_PACKED(event)->size; i++) {
                printf(" %x", LITL_READ_PACKED(event)->param[i]);
            }
            break;
        }
        case LITL_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n",
                    LITL_READ_GET_TYPE(event));
            *(int*) 0 = 0;
        }
        }

        printf("\n");
    }

    litl_read_finalize_trace(trace);

    return EXIT_SUCCESS;
}

