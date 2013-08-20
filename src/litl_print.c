/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <string.h>
#include <stdlib.h>

#include "litl_tools.h"
#include "litl_read.h"

static char* __input_filename = "trace";

static void __litl_read_usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr, "Usage: %s [-f input_filename] \n", argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __litl_read_parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-f") == 0)) {
            __input_filename = argv[++i];
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __litl_read_usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __litl_read_usage(argc, argv);
            exit(-1);
        }
    }

    if (strcmp(__input_filename, "trace") == 0) {
        __litl_read_usage(argc, argv);
        exit(-1);
    }
}

int main(int argc, char **argv) {
    litl_med_size_t i;
    litl_read_t* event;
    litl_trace_read_t *arch;
    litl_header_t* header;

    // parse the arguments passed to this program
    __litl_read_parse_args(argc, argv);

    arch = litl_read_open_trace(__input_filename);

    litl_read_init_traces(arch);

    header = litl_read_get_trace_header(arch);

    // print the header
    printf(" LiTL v.%s\n", header->litl_ver);
    printf(" %s\n", header->sysinfo);
    if (arch->header->is_trace_archive)
        printf(" nb_traces \t %d\n", arch->nb_traces);
    else
        printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n",
            header->buffer_size - __litl_get_reg_event_size(LITL_MAX_PARAMS)
                    - __litl_get_reg_event_size(0));

    printf(
            "[Timestamp]\t[ThreadID]\t[EventType]\t[EventCode]\t[NbParam]\t[Parameters]\n");
    while (1) {
        event = litl_read_next_event(arch);

        if (event == NULL )
            break;

        switch (LITL_READ_GET_TYPE(event)) {
        case LITL_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu32,
                    LITL_READ_GET_TIME(event), LITL_READ_GET_TID(event),
                    LITL_READ_GET_CODE(event),
                    LITL_READ_REGULAR(event)->nb_params);

            for (i = 0; i < LITL_READ_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, LITL_READ_REGULAR(event)->param[i]);
            break;
        }
        case LITL_TYPE_RAW: { // raw event
            printf("%"PRTIu64"\t%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu32,
                    LITL_READ_GET_TIME(event), LITL_READ_GET_TID(event),
                    LITL_READ_GET_CODE(event), LITL_READ_RAW(event)->size);
            printf("\t %s", (litl_data_t *) LITL_READ_RAW(event)->data);
            break;
        }
        case LITL_TYPE_PACKED: { // packed event
            printf(
                    "%"PRTIu64" \t%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu32"\t",
                    LITL_READ_GET_TIME(event), LITL_READ_GET_TID(event),
                    LITL_READ_GET_CODE(event), LITL_READ_PACKED(event)->size);
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
            abort();
        }
        }

        printf("\n");
    }

    litl_read_finalize_trace(arch);

    return EXIT_SUCCESS;
}
