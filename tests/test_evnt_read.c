/*
 * test_evnt_read.c
 *
 *  Created on: Nov 6, 2012
 *      Author: Roman Iakymchuk
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
    evnt_t* event;
    evnt_trace_t buffer;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "test_evnt_write.trace";

    buffer = open_trace(filename);
    printf("=============================================================\n");
    printf("Printing events from the %s file\n\n", filename);
    printf("Event Code \t Thread ID \t Time \t NB args \t Arguments (0-9)\n");

    while (buffer != NULL ) {
        event = read_event(&buffer);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            // regular event
            printf("%lx \t %lu \t %lu \t %lu", event->code, event->tid, event->time, event->nb_params);

            for (i = 0; i < event->nb_params; i++)
                printf("\t %lu", event->param[i]);
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%lx \t %lu \t %lu \t %lu", event->code, event->tid, event->time, event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    printf("=============================================================\n");
    close_trace(&buffer);

    return EXIT_SUCCESS;
}
