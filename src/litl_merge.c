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

static litl_trace_merge_t* __arch;
static litl_header_triples_t** __offsets;

/*
 * Sets a new name for the archive
 */
static void __litl_set_archive_name(const char* filename) {
    int res __attribute__ ((__unused__));

    // check whether the file name was set. If no, set it by default trace name
    if (filename == NULL )
        res = asprintf(&__arch->filename, "/tmp/%s_%s", getenv("USER"),
                "litl_archive_1");

    if (asprintf(&__arch->filename, "%s", filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Adds a trace header:
 *   - The number of traces
 *   - Triples: a file id, a file size, and an offset
 */
static void __add_archive_header() {
    int trace_in, res __attribute__ ((__unused__));
    litl_trace_size_t trace_size;
    litl_med_size_t i, j, header_size, header_size_global, nb_traces,
            nb_traces_global, triples_size;
    litl_buffer_t header_buffer, header_buffer_ptr;

    // add nb_traces and the is_trace_archive flag
    ((litl_header_t *) __arch->buffer)->nb_threads = __arch->nb_traces;
    ((litl_header_t *) __arch->buffer)->is_trace_archive = 1;
    header_size_global = sizeof(litl_data_t) + sizeof(litl_med_size_t);
    __arch->buffer += header_size_global;

    // create a array of arrays of offsets
    __offsets = (litl_header_triples_t **) malloc(
            __arch->nb_traces * sizeof(litl_header_triples_t *));

    nb_traces_global = 0;
    triples_size = sizeof(litl_header_triples_t);

    // read all header of traces and write them to the global header of the archive
    for (i = 0; i < __arch->nb_traces; i++) {
        if ((trace_in = open(__arch->trace_names[i], O_RDONLY)) < 0) {
            fprintf(stderr, "[litl_merge] Cannot open %s to read its header\n",
                    __arch->trace_names[i]);
            exit(EXIT_FAILURE);
        }

        header_size = sizeof(litl_data_t) + sizeof(litl_med_size_t);
        header_buffer_ptr = (litl_buffer_t) malloc(header_size);
        header_buffer = header_buffer_ptr;

        // read the header
        res = read(trace_in, header_buffer, header_size);
        if (((litl_header_t *) header_buffer)->is_trace_archive) {
            nb_traces = ((litl_header_t *) header_buffer)->nb_threads;
            __offsets[i] = (litl_header_triples_t *) malloc(
                    nb_traces * sizeof(litl_header_triples_t));

            // read triples
            header_size = nb_traces * sizeof(litl_header_triples_t);
            header_buffer_ptr = (litl_buffer_t) realloc(header_buffer_ptr,
                    header_size);
            header_buffer = header_buffer_ptr;

            res = read(trace_in, header_buffer, header_size);
            for (j = 0; j < nb_traces; j++) {
                trace_size =
                        ((litl_header_triples_t *) header_buffer)->trace_size;

                ((litl_header_triples_t *) __arch->buffer)->fid =
                        nb_traces_global;
                ((litl_header_triples_t *) __arch->buffer)->trace_size =
                        trace_size;
                __arch->buffer += triples_size;
                header_size_global += triples_size;

                __offsets[i][j].fid = nb_traces;
                __offsets[i][j].trace_size = trace_size;
                __offsets[i][j].offset =
                        ((litl_header_triples_t *) header_buffer)->offset;

                nb_traces_global++;
                header_buffer += triples_size;
            }
        } else {
            // move the pointer to the beginning of the trace
            lseek(trace_in, 0, SEEK_SET);

            struct stat st;
            if (fstat(trace_in, &st)) {
                perror("Cannot apply fstat to the input trace files!");
                exit(EXIT_FAILURE);
            }
            trace_size = st.st_size;

            ((litl_header_triples_t *) __arch->buffer)->fid = nb_traces_global;
            ((litl_header_triples_t *) __arch->buffer)->trace_size = trace_size;
            __arch->buffer += triples_size;
            header_size_global += triples_size;

            __offsets[i] = (litl_header_triples_t *) malloc(
                    sizeof(litl_header_triples_t));
            __offsets[i][0].fid = 1;
            __offsets[i][0].trace_size = trace_size;
            __offsets[i][0].offset = 0;

            nb_traces_global++;
        }

        free(header_buffer_ptr);
        close(trace_in);
    }

    // update the number of traces
    ((litl_header_t *) __arch->buffer_ptr)->nb_threads = nb_traces_global;

    res = write(__arch->f_handle, __arch->buffer_ptr, header_size_global);
    __arch->general_offset += header_size_global;
}

/*
 * Creates and opens an archive for traces.
 * Allocates memory for the buffer
 */
static void __litl_init_archive(const char* arch_name, char** trace_names,
        const int nb_traces) {

    __arch = (litl_trace_merge_t *) malloc(sizeof(litl_trace_merge_t));

    // allocate buffer for read/write ops
    __arch->buffer_size = 16 * 1024 * 1024; // 16 MB
    __arch->buffer_ptr = (litl_buffer_t) malloc(__arch->buffer_size);
    __arch->buffer = __arch->buffer_ptr;

    __arch->nb_traces = nb_traces;
    __arch->trace_names = trace_names;
    __arch->general_offset = 0;

    __litl_set_archive_name(arch_name);

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((__arch->f_handle = open(__arch->filename, O_WRONLY | O_CREAT, 0644))
            < 0) {
        fprintf(stderr, "[litl_merge] Cannot open %s archive\n",
                __arch->filename);
        exit(EXIT_FAILURE);
    }

    // write header with #traces and triples (fid, offset, traces_size)
    __add_archive_header();
}

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
static void __litl_create_archive() {
    int trace_in, res;
    litl_med_size_t i, j, triples_size, nb_traces;
    litl_trace_size_t trace_size, buffer_size, header_offset;

    triples_size = sizeof(litl_header_triples_t);
    // size of nb_traces and is_archive
    header_offset = sizeof(litl_data_t) + sizeof(litl_med_size_t);
    // size of fid and trace_size
    header_offset = header_offset + triples_size - sizeof(litl_offset_t);

    for (i = 0; i < __arch->nb_traces; i++) {
        if ((trace_in = open(__arch->trace_names[i], O_RDONLY)) < 0) {
            fprintf(stderr, "[litl_merge] Cannot open %s\n",
                    __arch->trace_names[i]);
            exit(EXIT_FAILURE);
        }

        // merge traces
        nb_traces = __offsets[i][0].fid;
        for (j = 0; j < nb_traces; j++) {
            trace_size = __offsets[i][j].trace_size;

            // update the offset in the global header
            lseek(__arch->f_handle, header_offset, SEEK_SET);
            res = write(__arch->f_handle, &__arch->general_offset,
                    sizeof(litl_offset_t));
            lseek(__arch->f_handle, __arch->general_offset, SEEK_SET);
            header_offset += triples_size;

            lseek(trace_in, __offsets[i][j].offset, SEEK_SET);

            // solution: Reading and writing blocks of data. Use the file size
            //           to deal with the reading of the last block from the
            //           traces
            while (trace_size > 0) {
                buffer_size =
                        trace_size > __arch->buffer_size ? __arch->buffer_size :
                                trace_size;

                res = read(trace_in, __arch->buffer, buffer_size);

                if (res < 0) {
                    perror("Cannot read the data from the traces!");
                    exit(EXIT_FAILURE);
                }

                res = write(__arch->f_handle, __arch->buffer, res);
                __arch->general_offset += res;

                trace_size -= buffer_size;
            }
        }

        close(trace_in);
    }

}

/*
 * Frees the allocated memory
 */
static void __litl_finalize_archive() {
    close(__arch->f_handle);

    // free offsets
    litl_med_size_t i;
    for (i = 0; i < __arch->nb_traces; i++)
        free(__offsets[i]);
    free(__offsets);

    // free filenames
    for (i = 0; i < __arch->nb_traces; i++)
        free(__arch->trace_names[i]);
    free(__arch->trace_names);

    free(__arch->buffer_ptr);

    __arch->buffer_ptr = NULL;
    __arch = NULL;
}

void litl_merge_traces(const char* arch_name, char** trace_names,
        const int nb_traces) {
    __litl_init_archive(arch_name, trace_names, nb_traces);

    __litl_create_archive();

    __litl_finalize_archive();
}
