/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_split.h"

static litl_trace_split_t* __arch;

/*
 * Opens an archive of traces
 */
static void __open_archive(const char *arch_name) {
    __arch = malloc(sizeof(litl_trace_split_t));

    // open an archive of traces
    if ((__arch->f_arch = open(arch_name, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", arch_name);
        exit(EXIT_FAILURE);
    }

    // allocate buffer for read/write ops
    __arch->buffer_size = 16 * 1024 * 1024; // 16 MB
    __arch->buffer = (litl_buffer_t) malloc(__arch->buffer_size);
}

/*
 * Initializes a structure that stores triples from the archive's header
 */
static void __read_archive_header() {
    int res __attribute__ ((__unused__));

    // at first, the header is small 'cause it stores only nb_traces and
    //   is_trace_archive values
    __arch->header_size = sizeof(litl_data_t) + sizeof(litl_med_size_t);
    __arch->header_buffer_ptr = (litl_buffer_t) malloc(__arch->header_size);
    __arch->header_buffer = __arch->header_buffer_ptr;

    // read the archive's header
    res = read(__arch->f_arch, __arch->header_buffer, __arch->header_size);

    // get the number of traces
    __arch->is_trace_archive =
            ((litl_general_header_t *) __arch->header_buffer)->is_trace_archive;
    __arch->nb_traces = ((litl_general_header_t *) __arch->header_buffer)->nb_threads;

    if (__arch->is_trace_archive == 0) {
        printf(
                "The given trace is not an archive. Therefore, there is nothing to split.\n");
        exit(EXIT_SUCCESS);
    }

    // Yes, we work with an archive of trace. So, we increase the header size
    //   and relocate the header buffer
    __arch->header_size = __arch->nb_traces * sizeof(litl_trace_triples_t);
    __arch->header_buffer_ptr = (litl_buffer_t) realloc(
            __arch->header_buffer_ptr, __arch->header_size);
    __arch->header_buffer = __arch->header_buffer_ptr;

    // read all triples
    res = read(__arch->f_arch, __arch->header_buffer, __arch->header_size);
}

/*
 * Writes each trace from an archive into a separate trace file
 */
static void __litl_split_archive(const char *dir) {
    int trace_out, res __attribute__ ((__unused__));
    char *trace_name = NULL;
    char user[32];
    litl_med_size_t size;

    size = sizeof(litl_trace_triples_t);

    strcpy(user, getenv("USER"));

    while (__arch->nb_traces-- != 0) {
        // get the triples
        __arch->triples = (litl_trace_triples_t *) __arch->header_buffer;
        __arch->header_buffer += size;

        // set a file pointer to the position of the current trace
        lseek(__arch->f_arch, __arch->triples->offset, SEEK_SET);

        // create and open a new trace file
        res = asprintf(&trace_name, "%s/%s_litl_log_rank_%d", dir, user,
                __arch->triples->fid);
        if ((trace_out = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
            fprintf(stderr, "Cannot open %s\n", trace_name);
            exit(EXIT_FAILURE);
        }

        // read data and write to separate traces
        while (1) {
            res = read(__arch->f_arch, __arch->buffer, __arch->buffer_size);

            if (__arch->triples->trace_size > __arch->buffer_size)
                res = write(trace_out, __arch->buffer, __arch->buffer_size);
            else {
                res = write(trace_out, __arch->buffer,
                        __arch->triples->trace_size);
                break;
            }

            __arch->triples->trace_size -= __arch->buffer_size;
        }

        free(trace_name);
        trace_name = NULL;
    }
}

/*
 * Closes the archive and free the allocated memory
 */
static void __close_archive() {
    close(__arch->f_arch);
    __arch->f_arch = -1;

    free(__arch->header_buffer_ptr);
    free(__arch->buffer);
    free(__arch);
}

void litl_split_archive(const char *arch_name, const char *output_dir) {
    // open an archive of traces and allocate memory for a buffer
    __open_archive(arch_name);

    // get info from the archive's header
    __read_archive_header();

    // split the archive
    __litl_split_archive(output_dir);

    // close the archive and free the allocated memory
    __close_archive();
}
