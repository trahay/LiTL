/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "litl_merge.h"

static char* __arch_name;
static litl_med_size_t __nb_traces = 0;
static struct litl_queue_t __trace_queue;

static void __usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr,
            "Usage: %s [-o archive_name] input_filename input_filename ... \n",
            argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
    int i, res __attribute__ ((__unused__));

    litl_init_queue(&__trace_queue);
    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-o") == 0)) {
            res = asprintf(&__arch_name, "%s", argv[++i]);
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __usage(argc, argv);
            exit(-1);
        } else {
            litl_enqueue(&__trace_queue, argv[i]);
            __nb_traces++;
        }
    }

    if (__arch_name == NULL)
        __usage(argc, argv);
}

int main(int argc, char **argv) {

    // parse the arguments passed to this program
    __parse_args(argc, argv);

    // init a buffer and an archive of traces
    litl_trace_merge_t arch = litl_init_archive(__arch_name, __nb_traces,
            &__trace_queue);

    // merging the trace files
    int i = 0;
    char *filename;
    while ((filename = litl_dequeue(&__trace_queue)) != NULL)
        litl_merge_file(&arch, i++, filename);

    // finalizing merging
    litl_finalize_archive(&arch);

    return EXIT_SUCCESS;
}
