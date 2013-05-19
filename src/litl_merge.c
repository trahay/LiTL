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

static evnt_buffer_t __buffer;
static int __buf_size = 16 * 1024 * 1024; // 16 MB

// offset from the beginning of the trace header
static evnt_offset_t __header_offset;
// offset from the beginning of the trace file until the current position
static evnt_offset_t __general_offset;

/*
 * Create and open an archive for traces
 * Allocate memory for the buffer
 */
static int __init_trace(char *trace_name) {
    int trace_out;

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((trace_out = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", trace_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    __buffer = (evnt_buffer_t) malloc(__buf_size);
    __header_offset = 0;
    __general_offset = 0;

    return trace_out;
}

/*
 * This function adds a trace header:
 *   - The number of traces
 *   - Triples: a file id, a file size, and an offset
 */
static void __add_trace_header(evnt_size_t nb_traces, const int trace_out) {
    evnt_size_t header_size;

    ((evnt_header_t *) __buffer)->nb_threads = nb_traces;
    // we do not add all the information about each trace, it will be added during packing them
    header_size = sizeof(evnt_size_t);
    __header_offset += header_size;
    header_size += nb_traces * (sizeof(evnt_size_t) + sizeof(evnt_file_size_t) + sizeof(evnt_offset_t));

    write(trace_out, __buffer, header_size);
    __general_offset += header_size;
}

/*
 * This function for merging trace files is a modified version of the cat implementation from the Kernighan & Ritchie book
 */
void litl_merge_file(const int file_id, const char *file_name_in, const int trace_out) {
    int trace_in, res;
    evnt_file_size_t file_size;

    if ((trace_in = open(file_name_in, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", file_name_in);
        exit(EXIT_FAILURE);
    }

    // find the trace size
    struct stat st;
    if (fstat(trace_in, &st)) {
        perror("Cannot apply fstat to the input trace files!");
        exit(EXIT_FAILURE);
    }
    file_size = st.st_size;

    // add triples (fid, file_size, offset)
    // add the pair tid and add & update offset at once
    lseek(trace_out, __header_offset, SEEK_SET);
    write(trace_out, &file_id, sizeof(evnt_size_t));
    write(trace_out, &file_size, sizeof(evnt_file_size_t));
    write(trace_out, &__general_offset, sizeof(evnt_offset_t));
    lseek(trace_out, __general_offset, SEEK_SET);
    __header_offset += sizeof(evnt_size_t) + sizeof(evnt_file_size_t) + sizeof(evnt_offset_t);

    // solution A: Reading and writing blocks of data. Use the file size to deal with the reading of the last block from
    //             the traces
    while ((res = read(trace_in, __buffer, __buf_size)) != 0) {
        if (res < 0) {
            perror("Cannot read the data from the traces!");
            exit(EXIT_FAILURE);
        }

        write(trace_out, __buffer, res);
        __general_offset += res;
    }

    /*// Solution B: Reading and writing characters
     int c;
     while ((c = getc(in)) != EOF)
     putc(c, out);*/

    close(trace_in);
}

static void __finalize_trace(int trace_out) {
    free(__buffer);
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
    int trace_out = __init_trace(file_name_out);

    // write header with #traces and reserved space for pairs (fid, offset)
    __add_trace_header(argc - 3, trace_out);

    // merging the trace files
    // TODO: 2 needs to be changed when the "-o trace_name_out" is right after "litl_merge"
    // TODO: use more meaningful file_id
    while (--argc > 2)
        litl_merge_file(argc, *++argv, trace_out);

    /*// error handling: valid ONLY for FILE*
     if (ferror(trace_out)) {
     fprintf(stderr, "%s: error while merging trace files\n", prog);
     exit(EXIT_FAILURE);
     }*/

    // finalizing merging
    __finalize_trace(trace_out);

    return EXIT_SUCCESS;
}
