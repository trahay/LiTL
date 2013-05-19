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

static evnt_buffer_t buffer;
static int buf_size = 16 * 1024 * 1024; // 16 MB

static int init(char *trace_name) {
    int trace_out;

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((trace_out = open(trace_name, O_WRONLY|O_CREAT, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", trace_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    buffer = (evnt_buffer_t) malloc(buf_size);

    return trace_out;
}

/*
 * This function for merging trace files is a modified version of the cat implementation from the Kernighan & Ritchie book
 */
void litl_merge_file(const char *file_name_in, const int trace_out) {
    int trace_in;

    if ((trace_in = open(file_name_in, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", file_name_in);
        exit(EXIT_FAILURE);
    }

    // solution A: Reading and writing blocks of data. Use the file size to deal with the reading of the last block from
    //             the traces
    evnt_size_t file_size;
    struct stat st;
    if (fstat(trace_in, &st)) {
        perror("Cannot apply fstat to the input trace files!");
        exit(EXIT_FAILURE);
    }
    file_size = st.st_size;

    int res;
    while ((res = read(trace_in, buffer, buf_size)) != 0) {
        if (res < 0) {
            perror("Cannot read the data from the traces!");
            exit(EXIT_FAILURE);
        }

        if (file_size < buf_size)
            write(trace_out, buffer, file_size);
        else {
            write(trace_out, buffer, buf_size);
            file_size -= buf_size;
        }
    }

    /*// Solution B: Reading and writing characters
     int c;
     while ((c = getc(in)) != EOF)
     putc(c, out);*/

    close(trace_in);
}

static void finalize(int trace_out) {
    free(buffer);
    close(trace_out);
}

int main(int argc, const char **argv) {
    char *prog, *file_name_out;

    // TODO: check for the arguments
    prog = argv[0];
    file_name_out = argv[argc - 1];
    /*if (strcmp(argv[argc - 2], ">")) {
     perror("Specify an archive trace file leading by the '>' character!");
     exit(EXIT_FAILURE);
     }*/

    // init a buffer and an archive of traces
    int trace_out = init(file_name_out);

    // TODO: write header with #traces and reserved space for pairs (fid, offset)

    // merging the trace files
    while (--argc > 1)
        litl_merge_file(*++argv, trace_out);

    /*// error handling: valid ONLY for FILE*
    if (ferror(trace_out)) {
        fprintf(stderr, "%s: error while merging trace files\n", prog);
        exit(EXIT_FAILURE);
    }*/

    // finalizing merging
    finalize(trace_out);

    return EXIT_SUCCESS;
}
