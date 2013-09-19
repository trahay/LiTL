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
static void __litl_split_open_archive(const char *arch_name) {
  __arch = malloc(sizeof(litl_trace_split_t));

  // open an archive of traces
  if ((__arch->f_handle = open(arch_name, O_RDONLY)) < 0) {
    fprintf(stderr, "Cannot open %s\n", arch_name);
    exit(EXIT_FAILURE);
  }

  // allocate buffer for read/write ops
  __arch->buffer_size = 16 * 1024 * 1024; // 16 MB
  __arch->buffer = (litl_buffer_t) malloc(__arch->buffer_size);
}

/*
 * Initializes a structure that stores triples from the trace header
 */
static void __litl_split_read_header() {
  int res __attribute__ ((__unused__));

  litl_size_t header_size, general_header_size;
  general_header_size = sizeof(litl_general_header_t);

  // read a general header
  header_size = general_header_size;
  __arch->header_buffer_ptr = (litl_buffer_t) malloc(header_size);
  res = read(__arch->f_handle, __arch->header_buffer_ptr, header_size);
  __arch->header_buffer = __arch->header_buffer_ptr;

  // get the number of processes
  __arch->trace_header = (litl_general_header_t *) __arch->header_buffer;
  __arch->nb_processes = __arch->trace_header->nb_processes;

  if (__arch->nb_processes == 1) {
    printf(
        "The given trace is not an archive. Therefore, there is nothing to split.\n");
    exit(EXIT_SUCCESS);
  }

  // Yes, we work with an archive of trace. So, we increase the header size
  //   and relocate the header buffer
  header_size += __arch->nb_processes * sizeof(litl_process_header_t);
  __arch->header_buffer_ptr = (litl_buffer_t) realloc(__arch->header_buffer_ptr,
                                                      header_size);

  // read headers of all processes
  res = read(__arch->f_handle, __arch->header_buffer_ptr + general_header_size,
             header_size - general_header_size);
  if (res == -1) {
    perror("Could not read the archive header!");
    exit(EXIT_FAILURE);
  }
  __arch->header_buffer = __arch->header_buffer_ptr;
  __arch->trace_header = (litl_general_header_t *) __arch->header_buffer;
  __arch->header_buffer += general_header_size;

  // for splitting: one process into one trace file
  __arch->trace_header->nb_processes = 1;
}

/*
 * Writes each trace from an archive into a separate trace file
 */
static void __litl_split_extract_traces(const char *dir) {
  int trace_out, res __attribute__ ((__unused__));
  char* trace_name;
  litl_size_t buffer_size;
  litl_med_size_t general_header_size, process_header_size;

  general_header_size = sizeof(litl_general_header_t);
  process_header_size = sizeof(litl_process_header_t);

  while (__arch->nb_processes-- != 0) {
    // get a process header
    __arch->process_header = (litl_process_header_t *) __arch->header_buffer;
    __arch->header_buffer += process_header_size;

    res = asprintf(&trace_name, "%s/%s", dir,
                   __arch->process_header->process_name);

    // create and open a new trace file
    if ((trace_out = open(trace_name, O_WRONLY | O_CREAT, 0644)) < 0) {
      fprintf(stderr, "Cannot create and open %s\n",
              __arch->process_header->process_name);
      exit(EXIT_FAILURE);
    }

    // write a general trace header
    res = write(trace_out, __arch->trace_header, general_header_size);

    // write a process header
    buffer_size = __arch->process_header->offset;
    __arch->process_header->offset = general_header_size + process_header_size;
    res = write(trace_out, __arch->process_header, process_header_size);

    // set a file pointer to the position of the current process
    lseek(__arch->f_handle, buffer_size, SEEK_SET);

    // read data and write to a separate trace
    while (__arch->process_header->trace_size) {
      buffer_size =
          __arch->process_header->trace_size > __arch->buffer_size ?
            __arch->buffer_size : __arch->process_header->trace_size;

      res = read(__arch->f_handle, __arch->buffer, buffer_size);

      res = write(trace_out, __arch->buffer, buffer_size);

      __arch->process_header->trace_size -= buffer_size;
    }

    free(trace_name);
    close(trace_out);
  }
}

/*
 * Closes the archive and free the allocated memory
 */
static void __litl_split_close_archive() {
  close(__arch->f_handle);
  __arch->f_handle = -1;

  free(__arch->header_buffer_ptr);
  free(__arch->buffer);
  free(__arch);
}

void litl_split_archive(const char *arch_name, const char *output_dir) {

  // open an archive of traces and allocate memory for a buffer
  __litl_split_open_archive(arch_name);

  // get info from the archive's header
  __litl_split_read_header();

  // split the archive
  __litl_split_extract_traces(output_dir);

  // close the archive and free the allocated memory
  __litl_split_close_archive();
}
