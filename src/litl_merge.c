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
static litl_trace_triples_t** __triples;

/*
 * Sets a new name for the archive
 */
static void __litl_merge_set_archive_name(const char* filename) {
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
static void __litl_merge_add_archive_header() {

    int trace_in, res __attribute__ ((__unused__));
    litl_med_size_t j, trace_index, header_size, general_header_size,
            process_header_size, global_header_size, nb_processes,
            total_nb_processes;
    litl_buffer_t header_buffer, header_buffer_ptr;

    total_nb_processes = 0;
    global_header_size = 0;
    general_header_size = sizeof(litl_general_header_t);
    process_header_size = sizeof(litl_process_header_t);

    // create an array of arrays of offsets
    __triples = (litl_trace_triples_t **) malloc(
            __arch->nb_traces * sizeof(litl_trace_triples_t *));

    // read all header of traces and write them to the global header of the archive
    for (trace_index = 0; trace_index < __arch->nb_traces; trace_index++) {

        if ((trace_in = open(__arch->traces_names[trace_index], O_RDONLY))
                < 0) {
            fprintf(stderr, "[litl_merge] Cannot open %s to read its header\n",
                    __arch->traces_names[trace_index]);
            exit(EXIT_FAILURE);
        }

        // read the trace header
        header_buffer_ptr = (litl_buffer_t) malloc(general_header_size);
        res = read(trace_in, header_buffer_ptr, general_header_size);
        header_buffer = header_buffer_ptr;

        nb_processes = ((litl_general_header_t *) header_buffer)->nb_processes;
        __triples[trace_index] = (litl_trace_triples_t *) malloc(
                nb_processes * sizeof(litl_trace_triples_t));

        // add a general header
        if (trace_index == 0) {
            sprintf(
                    (char*) ((litl_general_header_t *) __arch->buffer)->litl_ver,
                    "%s",
                    (char*) ((litl_general_header_t *) header_buffer)->litl_ver);
            sprintf((char*) ((litl_general_header_t *) __arch->buffer)->sysinfo,
                    "%s",
                    (char*) ((litl_general_header_t *) header_buffer)->sysinfo);

            global_header_size += general_header_size;
            __arch->buffer += general_header_size;
        }

        // read headers of processes
        header_size = nb_processes * process_header_size;
        res = read(trace_in, __arch->buffer, header_size);

        // find the trace size
        if (nb_processes == 1) {
            // lseek(trace_in, 0, SEEK_SET);
            struct stat st;
            if (fstat(trace_in, &st)) {
                perror("Cannot apply fstat to the input trace files!");
                exit(EXIT_FAILURE);
            }

            ((litl_process_header_t *) __arch->buffer)->trace_size =
                    (litl_trace_size_t) st.st_size - general_header_size
                            - process_header_size;
        }

        for (j = 0; j < nb_processes; j++) {
            __triples[trace_index][j].nb_processes = nb_processes;
            __triples[trace_index][j].offset = global_header_size
                    + (j + 1) * process_header_size - sizeof(litl_offset_t);
        }

        total_nb_processes += nb_processes;
        global_header_size += header_size;
        __arch->buffer += header_size;

        free(header_buffer_ptr);
        close(trace_in);
    }

    // update the number of processes
    ((litl_general_header_t *) __arch->buffer_ptr)->nb_processes =
            total_nb_processes;

    res = write(__arch->f_handle, __arch->buffer_ptr, global_header_size);
    __arch->general_offset += global_header_size;
}

/*
 * Creates and opens an archive for traces.
 * Allocates memory for the buffer
 */
static void __litl_merge_init_archive(const char* arch_name,
        char** traces_names, const int nb_traces) {

    __arch = (litl_trace_merge_t *) malloc(sizeof(litl_trace_merge_t));

    // allocate buffer for read/write ops
    __arch->buffer_size = 16 * 1024 * 1024; // 16 MB
    __arch->buffer_ptr = (litl_buffer_t) malloc(__arch->buffer_size);
    __arch->buffer = __arch->buffer_ptr;

    __arch->nb_traces = nb_traces;
    __arch->traces_names = traces_names;
    __arch->general_offset = 0;

    __litl_merge_set_archive_name(arch_name);

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((__arch->f_handle = open(__arch->filename, O_WRONLY | O_CREAT, 0644))
            < 0) {
        fprintf(stderr, "[litl_merge] Cannot open %s archive\n",
                __arch->filename);
        exit(EXIT_FAILURE);
    }

    // add a general archive header and also a set of process headers
    __litl_merge_add_archive_header();
}

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
static void __litl_merge_create_archive() {
    int trace_in, res;
    litl_med_size_t j, trace_index, nb_processes;
    litl_trace_size_t buffer_size, header_offset, general_header_size,
            process_header_size;

    general_header_size = sizeof(litl_general_header_t);
    process_header_size = sizeof(litl_process_header_t);

    for (trace_index = 0; trace_index < __arch->nb_traces; trace_index++) {
        if ((trace_in = open(__arch->traces_names[trace_index], O_RDONLY))
                < 0) {
            fprintf(stderr, "[litl_merge] Cannot open %s\n",
                    __arch->traces_names[trace_index]);
            exit(EXIT_FAILURE);
        }

        // update offsets of processes
        nb_processes = __triples[trace_index][0].nb_processes;
        for (j = 0; j < nb_processes; j++) {
            // TODO: for multiple processes
            lseek(__arch->f_handle, __triples[trace_index][j].offset, SEEK_SET);
            res = write(__arch->f_handle, &__arch->general_offset,
                    sizeof(litl_offset_t));
            lseek(__arch->f_handle, __arch->general_offset, SEEK_SET);
        }

        // merge traces
        header_offset = general_header_size
                + nb_processes * process_header_size;
        lseek(trace_in, header_offset, SEEK_SET);

        // solution: Reading and writing blocks of data. Use the file size
        //           to deal with the reading of the last block from the
        //           traces
        while (1) {
            res = read(trace_in, __arch->buffer, __arch->buffer_size);

            if (res < 0) {
                perror("Cannot read the data from the traces!");
                exit(EXIT_FAILURE);
            }

            res = write(__arch->f_handle, __arch->buffer, res);
            __arch->general_offset += res;

            if (res < __arch->buffer_size)
                break;
        }

        close(trace_in);
    }
}

/*
 * Frees the allocated memory
 */
static void __litl_merge_finalize_archive() {
    close(__arch->f_handle);

    // free offsets
    litl_med_size_t i;
    for (i = 0; i < __arch->nb_traces; i++)
        free(__triples[i]);
    free(__triples);

    // free filenames
    for (i = 0; i < __arch->nb_traces; i++)
        free(__arch->traces_names[i]);
    free(__arch->traces_names);

    free(__arch->buffer_ptr);

    __arch->buffer_ptr = NULL;
    __arch = NULL;
}

void litl_merge_traces(const char* arch_name, char** traces_names,
        const int nb_traces) {
    __litl_merge_init_archive(arch_name, traces_names, nb_traces);

    __litl_merge_create_archive();

    __litl_merge_finalize_archive();
}
