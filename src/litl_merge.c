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

static struct litl_queue_t* __trace_queue;

/*
 * Sets a new name for the archive
 */
void litl_set_archive_name(litl_trace_merge_t* arch, char* filename) {
    // check whether the file name was set. If no, set it by default trace name.
    if (filename == NULL)
        sprintf(filename, "/tmp/%s_%s", getenv("USER"), "litl_archive_1");

    if (asprintf(&arch->filename, "%s", filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Adds a trace header:
 *   - The number of traces
 *   - Triples: a file id, a file size, and an offset
 */
static void __add_archive_header(litl_trace_merge_t *arch) {
    int res __attribute__ ((__unused__));
    litl_size_t header_size;

    // add nb_traces and the is_trace_archive flag
    ((litl_header_t *) arch->buffer)->nb_threads = arch->nb_traces;
    ((litl_header_t *) arch->buffer)->is_trace_archive = 1;

    header_size = sizeof(litl_tiny_size_t) + sizeof(litl_med_size_t);
    arch->header_offset += header_size;
    res = write(arch->f_handle, arch->buffer, header_size);

    // we do not add all the information about each trace 'cause it will be
    // added during packing them, instead we just reserve the space for that
    // +1 is for the offset for the next portion of triples
    header_size += (arch->nb_traces + 1) * sizeof(litl_header_triples_t);
    arch->general_offset += header_size;
    lseek(arch->f_handle, header_size, SEEK_SET);
}

/*
 * Creates and opens an archive for traces.
 * Allocates memory for the buffer
 */
litl_trace_merge_t litl_init_archive(char* arch_name, litl_size_t nb_traces,
        litl_queue_t* trace_queue) {
    litl_trace_merge_t arch;

    __trace_queue = trace_queue;

    // allocate buffer for read/write ops
    arch.buffer_size = 16 * 1024 * 1024; // 16 MB
    arch.buffer = (litl_buffer_t) malloc(arch.buffer_size);

    arch.nb_traces = nb_traces;
    arch.header_offset = 0;
    arch.general_offset = 0;

    litl_set_archive_name(&arch, arch_name);

    // create an archive for trace files in rw-r-r- mode (0644)
    if ((arch.f_handle = open(arch.filename, O_WRONLY | O_CREAT, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", arch.filename);
        exit(EXIT_FAILURE);
    }

    // write header with #traces and reserved space for
    //    triples (fid, offset, traces_size)
    __add_archive_header(&arch);

    return arch;
}

/*
 * Checks whether a trace is an archive or not.
 * If it is an archive, the number of compressed traces is returned
 */
static uint8_t __is_archive(int fid) {
    int header_size, nb_traces, res __attribute__ ((__unused__));

    header_size = sizeof(litl_tiny_size_t) + sizeof(litl_med_size_t);
    litl_buffer_t header_buffer = (litl_buffer_t) malloc(header_size);

    // read the header
    res = read(fid, header_buffer, header_size);

    nb_traces = 0;
    if (((litl_header_t *) header_buffer)->is_trace_archive)
        nb_traces = ((litl_header_t *) header_buffer)->nb_threads;
    else
        lseek(fid, 0, SEEK_SET);

    free(header_buffer);
    return nb_traces;
}

/*
 * Merges trace files. This is a modified version of the cat implementation
 *   from the Kernighan & Ritchie book
 */
void litl_merge_file(litl_trace_merge_t *arch, int file_id,
        const char *file_name_in) {
    int trace_in, res;
    litl_med_size_t nb_traces;
    litl_trace_size_t trace_size;

    if ((trace_in = open(file_name_in, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", file_name_in);
        exit(EXIT_FAILURE);
    }

    // get the trace header and check whether it is an archive or a trace

    nb_traces = __is_archive(trace_in);
    printf("nb_traces = %d\n", nb_traces);
    if (nb_traces) {
        // an archive of traces
        litl_med_size_t i, triples_size;
        litl_size_t buffer_size, header_size;
        litl_buffer_t header_buffer_ptr, header_buffer;
        header_size = nb_traces * sizeof(litl_header_triples_t);
        header_buffer_ptr = (litl_buffer_t) malloc(header_size);
        header_buffer = header_buffer_ptr;

        // reading triples
        triples_size = sizeof(litl_header_triples_t);
        res = read(trace_in, header_buffer, header_size);

        // read/write data of each file and add triples to the header or
        //   the other place
        file_id = file_id * 10;
        for (i = 0; i < nb_traces; i++) {
            file_id += i;
            trace_size = ((litl_header_triples_t *) header_buffer)->trace_size;
            printf("trace_size = %lu\n", trace_size);
            lseek(arch->f_handle, arch->header_offset, SEEK_SET);
            res = write(arch->f_handle, &file_id, sizeof(litl_med_size_t));
            res = write(arch->f_handle, &trace_size, sizeof(litl_trace_size_t));
            res = write(arch->f_handle, &arch->general_offset,
                    sizeof(litl_offset_t));
            lseek(arch->f_handle, arch->general_offset, SEEK_SET);
            arch->header_offset += triples_size;

            // solution: Reading and writing blocks of data. Use the file size
            //           to deal with the reading of the last block from the
            //           traces
            lseek(trace_in, ((litl_header_triples_t *) header_buffer)->offset,
                    SEEK_SET);
            buffer_size =
                    trace_size > arch->buffer_size ? arch->buffer_size :
                            trace_size;
            while (buffer_size) {
                res = read(trace_in, arch->buffer, buffer_size);

                if (res < 0) {
                    perror("Cannot read the data from the traces!");
                    exit(EXIT_FAILURE);
                }

                res = write(arch->f_handle, arch->buffer, res);
                arch->general_offset += res;

                trace_size -= buffer_size;
                buffer_size =
                        trace_size > arch->buffer_size ? arch->buffer_size :
                                trace_size;
            }

            header_buffer += triples_size;
            if (i)
                arch->nb_traces++;
        }
        free(header_buffer_ptr);

        // update the number of traces in the header by (nb_traces - 1)
        lseek(arch->f_handle, 0, SEEK_SET);
        res = write(arch->f_handle, &arch->nb_traces, sizeof(litl_med_size_t));
        lseek(arch->f_handle, arch->general_offset, SEEK_SET);
    } else {
        // a regular trace
        // find the trace size
        struct stat st;
        if (fstat(trace_in, &st)) {
            perror("Cannot apply fstat to the input trace files!");
            exit(EXIT_FAILURE);
        }
        trace_size = st.st_size;

        // add triples (fid, file_size, offset)
        lseek(arch->f_handle, arch->header_offset, SEEK_SET);
        res = write(arch->f_handle, &file_id, sizeof(litl_med_size_t));
        res = write(arch->f_handle, &trace_size, sizeof(litl_trace_size_t));
        res = write(arch->f_handle, &arch->general_offset,
                sizeof(litl_offset_t));
        lseek(arch->f_handle, arch->general_offset, SEEK_SET);
        arch->header_offset += sizeof(litl_header_triples_t);

        // solution: Reading and writing blocks of data. Use the file size to deal
        //           with the reading of the last block from the traces
        while ((res = read(trace_in, arch->buffer, arch->buffer_size)) != 0) {
            if (res < 0) {
                perror("Cannot read the data from the traces!");
                exit(EXIT_FAILURE);
            }

            res = write(arch->f_handle, arch->buffer, res);
            arch->general_offset += res;
        }
    }

    close(trace_in);
}

/*
 * Frees the allocated memory
 */
void litl_finalize_archive(litl_trace_merge_t *arch) {
    // remove the queue consisting of trace file names
    litl_delqueue(__trace_queue);

    close(arch->f_handle);

    free(arch->buffer);

    arch->buffer = NULL;
    arch = NULL;
}
