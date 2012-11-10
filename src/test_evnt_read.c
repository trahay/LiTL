/*
 * test_evnt_read.c
 *
 *  Created on: Nov 6, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdio.h>
#include <stdlib.h>

#include "evnt_types.h"
#include "evnt_read.h"

int main(int argc, const char * argv[]) {
    uint64_t i;
    char* filename = "test_evnt_write.trace";
    evnt* event;
    trace buffer;

    buffer = open_trace(filename);
    printf("=============================================================\n");
    printf("Printing events from the %s file\n\n", filename);
    printf("Event Code \t Thread ID \t Time \t NB args \t Arguments (0-9)\n");

    while (buffer != NULL ) {
        event = read_event(&buffer);

        if (event == NULL )
            break;

        printf("%lu \t %lu \t %lu \t %lu", event->code, event->tid, event->time, event->nb_args);

        for (i = 0; i < event->nb_args; i++)
            printf("\t %lu", event->args[i]);

        printf("\n");
    }

    printf("=============================================================\n");
    close_trace(&buffer);

    return EXIT_SUCCESS;
}
