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

#include "litl_queue.h"
#include "litl_merge.h"

static char* __archive_name = "archive";
static litl_trace_merge_t* __archive;
static litl_size_t __nb_traces = 0;
static struct litl_queue_t trace_queue;

static void __usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr,
            "Usage: %s [-o archive_name] input_filename input_filename ... \n",
            argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
    int i;

    litl_init_queue(&trace_queue);
    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-o") == 0)) {
            __archive_name = argv[++i];
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __usage(argc, argv);
            exit(-1);
        } else {
            litl_enqueue(&trace_queue, argv[i]);
            __nb_traces++;
        }
    }

    if (strcmp(__archive_name, "archive") == 0) {
        __usage(argc, argv);
        exit(-1);
    }
}

/*
 * Creates and opens an archive for traces.
 * Allocates memory for the buffer
 */
static void __init_trace(char *trace_name) {
    __archive = malloc(sizeof(litl_trace_merge_t));

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((__archive->f_arch = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", trace_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    __archive->buffer_size = 16 * 1024 * 1024; // 16 MB
    __archive->buffer = (litl_buffer_t) malloc(__archive->buffer_size);
    __archive->header_offset = 0;
    __archive->general_offset = 0;
}

/*
 * Adds a trace header:
 *   - The number of traces
 *   - Triples: a file id, a file size, and an offset
 */
static void __add_trace_header() {
    int res __attribute__ ((__unused__));
    litl_size_t header_size;

    // add nb_traces and the is_trace_archive flag
    ((litl_header_t *) __archive->buffer)->nb_threads = __nb_traces;
    ((litl_header_t *) __archive->buffer)->is_trace_archive = 1;
    header_size = sizeof(litl_size_t) + sizeof(litl_tiny_size_t);
    __archive->header_offset += header_size;
    res = write(__archive->f_arch, __archive->buffer, header_size);

    // we do not add all the information about each trace 'cause it will be
    // added during packing them, instead we just reserve the space for that
    header_size += __nb_traces * sizeof(litl_header_triples_t);
    __archive->general_offset += header_size;
    lseek(__archive->f_arch, header_size, SEEK_SET);
}

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
void litl_merge_file(const int file_id, const char *file_name_in) {
    int trace_in, res;
    litl_trace_size_t file_size;

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
    lseek(__archive->f_arch, __archive->header_offset, SEEK_SET);
    res = write(__archive->f_arch, &file_id, sizeof(litl_tid_t));
    res = write(__archive->f_arch, &file_size, sizeof(litl_trace_size_t));
    res = write(__archive->f_arch, &__archive->general_offset,
            sizeof(litl_offset_t));
    lseek(__archive->f_arch, __archive->general_offset, SEEK_SET);
    __archive->header_offset += sizeof(litl_header_triples_t);

    // solution A: Reading and writing blocks of data. Use the file size to deal
    //             with the reading of the last block from the traces
    while ((res = read(trace_in, __archive->buffer, __archive->buffer_size))
            != 0) {
        if (res < 0) {
            perror("Cannot read the data from the traces!");
            exit(EXIT_FAILURE);
        }

        res = write(__archive->f_arch, __archive->buffer, res);
        __archive->general_offset += res;
    }

    close(trace_in);
}

static void __finalize_trace() {
    // remove the queue consisting of trace file names
    litl_delqueue(&trace_queue);
    free(__archive->buffer);
    close(__archive->f_arch);
    free(__archive);
}

int main(int argc, char **argv) {

    // parse the arguments passed to this program
    __parse_args(argc, argv);

    // init a buffer and an archive of traces
    __init_trace(__archive_name);

    // write header with #traces and reserved space for
    //    triples (fid, offset, traces_size)
    __add_trace_header();

    // merging the trace files
    int i = 0;
    char *filename;
    while ((filename = litl_dequeue(&trace_queue)) != NULL )
        litl_merge_file(i++, filename);

    // finalizing merging
    __finalize_trace();

    return EXIT_SUCCESS;
}
