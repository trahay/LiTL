/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "evnt_types.h"
#include "evnt_write.h"

int main(int argc, const char **argv) {
    int i, nb_iter;

    evnt_trace_write_t trace;
    const char* filename = "trace";
    const uint32_t buffer_size = 32 * 1024; // 32KB

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else
        filename = "/tmp/test_trace_size.trace";

    printf("Recording events with six arguments of type uint8_t\n\n");

    trace = evnt_init_trace(buffer_size);
    evnt_set_filename(&trace, filename);

    nb_iter = 1000;
    for (i = 0; i < nb_iter; i++) {
        // event6
        /*        evnt_probe_pack_6(&trace, 0x100 * (i + 1) + 6, (uint8_t ) 1, (uint8_t ) 3, (uint8_t ) 5, (uint8_t ) 7,
         (uint8_t ) 11, (uint8_t ) 13);*/
        evnt_probe_pack_6(&trace, 0x100 * (i + 1) + 6, (int32_t ) 1, (int32_t ) 3, (int32_t ) 5, (int32_t ) 7,
                (int32_t ) 11, (int32_t ) 13);
        usleep(100);
    }

    evnt_fin_trace(&trace);

    printf("Events are recorded and written in the %s file\n", filename);

    evnt_param_t size;
    FILE* fp = fopen(filename, "r");
    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    printf("\nThe size of the trace file with %d packed event6 is %d bytes \n", nb_iter, size);

    return EXIT_SUCCESS;
}
