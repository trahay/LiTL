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
    const char* filename = "trace.trace";
    const evnt_size_t buffer_size = 16 * 1024 * 1024; // 16MB
    evnt_t* event;
    evnt_trace_t buffer;
    evnt_block_t block;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    set_read_buffer_size(buffer_size);
    buffer = open_trace(filename);
    block = get_evnt_block(buffer);
    printf("=============================================================\n");
    printf("Printing events from the %s file\n\n", filename);
    printf("Event Code \t Thread ID \t Time \t NB args \t Arguments[0-9]\n");

    while (block.trace != NULL ) {
        event = read_event(&block);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            // regular event
            printf("%"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, event->tid, event->time, event->nb_params);

            for (i = 0; i < event->nb_params; i++)
                printf("\t %"PRTIx64, event->param[i]);
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, event->tid, event->time, event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    printf("=============================================================\n");
    close_trace(&block);

    return EXIT_SUCCESS;
}
