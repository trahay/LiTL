/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include "litl_merge.h"

static FILE *trace_out;
static int buf_size = 16 * 1024 * 1024; // 16 MB
static evnt_buffer_t buffer;

static void init(char *trace_name) {
    // open an archive of trace files
    if ((trace_out = fopen(trace_name, "a")) == NULL ) {
        fprintf(stderr, "Cannot open %s\n", trace_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    //    buffer = (evnt_buffer_t) malloc(buf_size);
}

void litl_merge_file(FILE *in, FILE *out) {
/*    evnt_size_t res;

    // TODO: implement using only open, close, read, and write functions
    while ((res = fread(buffer, 1, buf_size, trace_out)) != 0)
        fwrite(buffer, 1, res, trace_out);*/
    int c;

    while ((c = getc(in)) != EOF)
        putc(c, out);
}

static void finilize() {
    free(trace_out);
}

int main(int argc, const char **argv) {
    FILE *trace_in;
    char *prog, *trace_out_name;

    // TODO: check for the arguments
    prog = argv[0];
    trace_out_name = argv[argc - 1];
    /*if (strcmp(argv[argc - 2], ">")) {
     perror("Specify an archive trace file leading by the '>' character!");
     exit(EXIT_FAILURE);
     }*/

    // init a buffer and an archive of traces
    init(trace_out_name);

    // TODO: write header with #traces and reserved space for pairs (fid, offset)

    // merging the trace files
    while (--argc > 1)
        if ((trace_in = fopen(*++argv, "r")) == NULL ) {
            fprintf(stderr, "Cannot open %s\n", *argv);
            exit(EXIT_FAILURE);
        } else {
            printf("%s\n", *argv);
            litl_merge_file(trace_in, trace_out);
            fclose(trace_in);
        }

    if (ferror(trace_out)) {
        fprintf(stderr, "%s: error while merging trace files\n", prog);
        exit(EXIT_FAILURE);
    }

    finilize();
    return EXIT_SUCCESS;
}
