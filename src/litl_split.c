/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "litl_split.h"

static litl_trace_split_t* __archive;

static char *__archive_name = "archive";
static char *__output_dir = "dir";

static void __usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr, "Usage: %s [-f archive_traces] [-d output_dir] \n", argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-f") == 0)) {
            __archive_name = argv[++i];
        } else if ((strcmp(argv[i], "-d") == 0)) {
            __output_dir = argv[++i];
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __usage(argc, argv);
            exit(-1);
        }
    }

    if (strcmp(__archive_name, "archive") == 0) {
        __usage(argc, argv);
        exit(-1);
    } else if (strcmp(__output_dir, "dir") == 0) {
        __usage(argc, argv);
        exit(-1);
    }
}

/*
 * Open an archive of traces
 */
static void __open_archive(const char *archive_name) {
    __archive = malloc(sizeof(litl_trace_split_t));

    // open an archive of traces
    if ((__archive->f_arch = open(archive_name, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", archive_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    __archive->buffer_size = 16 * 1024 * 1024; // 16 MB
    __archive->buffer = (litl_buffer_t) malloc(__archive->buffer_size);
}

/*
 * Initialize a structure that stores triples from the archive's header
 */
static void __read_archive_header() {
    // At first, the header is small 'cause it stores only nb_traces and is_trace_archive values
    __archive->header_size = sizeof(litl_size_t) + sizeof(litl_tiny_size_t);
    __archive->header_buffer = (litl_buffer_t) malloc(__archive->header_size);

    // read the archive's header
    read(__archive->f_arch, __archive->header_buffer, __archive->header_size);

    // get the number of traces
    __archive->header = (litl_header_t *) __archive->header_buffer;
    __archive->nb_traces = __archive->header->nb_threads;

    if (__archive->header->is_trace_archive == 0) {
        printf("The given trace is not an archive of traces. Therefore, there is nothing to be split.\n");
        exit(EXIT_SUCCESS);
    }

    // Yes, we work with an archive of trace. So, we increase the header size and relocate the header buffer
    __archive->header_size = __archive->nb_traces * sizeof(litl_header_triples_t); // +1 to allocate slightly more
    __archive->header_buffer = (litl_buffer_t) realloc(__archive->header_buffer, __archive->header_size);

    // read all triples
    read(__archive->f_arch, __archive->header_buffer, __archive->header_size);
}

/*
 * Write each trace from an archive into a separate trace file
 */
void litl_split_archive(const char *dir) {
    int trace_out;
    char *trace_name = NULL;
    char user[32];
    litl_size_t size;
    litl_offset_t trace_offset;

    size = sizeof(litl_header_triples_t);
    trace_offset = 0;

    strcpy(user, getenv("USER"));

    while (__archive->nb_traces-- != 0) {
        // get the triples
        __archive->triples = (litl_header_triples_t *) __archive->header_buffer;
        __archive->header_buffer += size;

        // set a file pointer to the position of the current trace
        lseek(__archive->f_arch, __archive->triples->offset, SEEK_SET);

        // create and open a new trace file
        asprintf(&trace_name, "%s/%s_eztrace_log_rank_%d", dir, user, __archive->triples->fid);
        if ((trace_out = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
            fprintf(stderr, "Cannot open %s\n", trace_name);
            exit(EXIT_FAILURE);
        }

        // read data and write to separate traces
        while (1) {
            read(__archive->f_arch, __archive->buffer, __archive->buffer_size);

            if (__archive->triples->trace_size > __archive->buffer_size)
                write(trace_out, __archive->buffer, __archive->buffer_size);
            else {
                write(trace_out, __archive->buffer, __archive->triples->trace_size);
                break;
            }

            __archive->triples->trace_size -= __archive->buffer_size;
        }

        free(trace_name);
        trace_name = NULL;
    }
}

/*
 * Close the archive and free the allocated memory
 */
static void __close_archive() {
    free(__archive->buffer);
    close(__archive->f_arch);
    free(__archive);
}

int main(int argc, const char **argv) {

    // parse the arguments passed to this program
    __parse_args(argc, argv);

    // open an archive of traces and allocate memory for a buffer
    __open_archive(__archive_name);

    // get info from the archive's header
    __read_archive_header();

    // split the archive
    litl_split_archive(__output_dir);

    // close the archive and free the allocated memory
    __close_archive();

    return EXIT_SUCCESS;
}
