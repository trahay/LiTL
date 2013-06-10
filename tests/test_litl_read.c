/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "litl_types.h"
#include "litl_read.h"
#include "litl_macro.h"

int main(int argc, const char **argv) {
    litl_size_t i, index;
    const char* filename = "trace";
    litl_read_t* event;
    litl_trace_read_t *trace;
    litl_header_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_litl_write.trace";

    trace = litl_open_trace(filename);
    header = litl_get_trace_header(trace);

    // print the header
    printf(" LiTL v.%s\n", header->liblitl_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    printf("Thread ID\t Event Type & Code \t Time\t   NB args\t Arguments[0-9]\n");
    while (1) {
        event = litl_next_buffer_event(trace, index);

        if (event == NULL)
            break;

        switch (LITL_GET_TYPE(event)) {
        case LITL_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, LITL_GET_TID(event), LITL_GET_CODE(event),
                    LITL_GET_TIME(event), LITL_REGULAR(event)->nb_params);

            for (i = 0; i < LITL_REGULAR(event)->nb_params; i++)
            printf("\t %"PRTIx64, LITL_REGULAR(event)->param[i]);
            break;
        }
        case LITL_TYPE_RAW: { // raw event
            LITL_GET_CODE(event) = clear_bit(LITL_GET_CODE(event));
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, LITL_GET_TID(event), LITL_GET_CODE(event),
                    LITL_GET_TIME(event), LITL_RAW(event)->size);
            printf("\t %s", (litl_data_t *) LITL_RAW(event)->data);
            break;
        }
        case LITL_TYPE_PACKED: { // packed event
            printf("%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_GET_TIME(event), LITL_PACKED(event)->size);
            for (i = 0; i < LITL_PACKED(event)->size; i++) {
                printf(" %x", LITL_PACKED(event)->param[i]);
            }
            break;
        }
        case LITL_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", LITL_GET_TYPE(event));
            *(int*) 0 = 0;
        }
        }

        printf("\n");
    }

    litl_close_trace(trace);

    return EXIT_SUCCESS;
}
