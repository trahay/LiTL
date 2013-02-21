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
    evnt_buffer_t buffer;
    evnt_block_t block;
    evnt_info_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    set_read_buffer_size(buffer_size);
    buffer = evnt_open_trace(filename);
    block = evnt_get_block(buffer);
    //    header = evnt_get_trace_header(&block);

    printf("=============================================================\n");
    printf("Printing events from the %s file\n\n", filename);

    /*// print the header
     printf("    libevnt v.%s\n", header->libevnt_ver);
     printf("    %s\n", header->sysinfo);
     printf("\n");*/

    printf("Event Code\t Thread ID\t Time\t\t   NB args\t Arguments[0-9]\n");

    while (block.buffer != NULL ) {
        event = evnt_read_event(&block);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            // regular event
            printf("%"PRTIx32"\t %"PRTIu64"\t %"PRTIu64"\t %"PRTIu32, event->code, event->tid, event->time,
                    event->nb_params);

            for (i = 0; i < event->nb_params; i++)
                printf("\t %"PRTIx64, event->param[i]);
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%"PRTIx32"\t %"PRTIu64"\t %"PRTIu64"\t %"PRTIu32, event->code, event->tid, event->time,
                    event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    printf("=============================================================\n");
    evnt_close_trace(&block);

    return EXIT_SUCCESS;
}
