/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_tools.h"
#include "litl_read.h"

/*
 * Initializes the archive header
 */
static void __litl_read_init_header(litl_read_trace_t* arch) {
    int res;

    // at first, the header is small 'cause it stores only nb_processes and
    //   is_trace_archive values
    arch->header_size = sizeof(litl_data_t) + sizeof(litl_med_size_t);
    arch->header_buffer_ptr = (litl_buffer_t) malloc(arch->header_size);
    if (!arch->header_buffer_ptr) {
        perror("Could not allocate memory for the header!");
        exit(EXIT_FAILURE);
    }

    // read the archive header
    res = read(arch->f_handle, arch->header_buffer_ptr, arch->header_size);
    // If the end of file is reached, then all data are read; res = 0.
    //   Otherwise, res equals the number of elements (= 1) or the error
    //   occurred and res = -1.
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    arch->header_buffer = arch->header_buffer_ptr;
    arch->header = (litl_general_header_t *) arch->header_buffer;

    // get the number of processes
    if (arch->header->is_trace_archive) {
        // Yes, we work with an archive of trace. So, we increase
        //   the header size and relocate the header buffer
        arch->nb_processes =
                ((litl_general_header_t *) arch->header_buffer)->nb_threads;
        arch->header_size = arch->nb_processes * sizeof(litl_process_triples_t);
        arch->header_buffer = (litl_buffer_t) realloc(arch->header_buffer_ptr,
                arch->header_size);

        // read the archive header
        res = read(arch->f_handle, arch->header_buffer_ptr, arch->header_size);
        arch->header_buffer = arch->header_buffer_ptr;
    } else {
        arch->nb_processes = 1;
    }
}

/*
 * Initializes the trace header, meaning it reads chunks with all pairs
 */
static void __litl_read_init_trace_header(litl_read_trace_t* arch,
        litl_read_process_events_t* trace) {

    // init the header structure
    // the minimum size: size of the header with all information and
    //   one pair (tid, offset)
    trace->header_size = sizeof(litl_general_header_t)
            + sizeof(litl_thread_pairs_t);
    trace->header_buffer_ptr = (litl_buffer_t) malloc(trace->header_size);
    if (!trace->header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    // read the header
    lseek(arch->f_handle, trace->triples->offset, SEEK_SET);
    int res = read(arch->f_handle, trace->header_buffer_ptr,
            trace->header_size);
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header = (litl_general_header_t *) trace->header_buffer_ptr;

    // check if the buffer is large enough to store the header
    if (trace->header->nb_threads > 1) {
        litl_med_size_t nb_threads =
                (trace->header->header_nb_threads > NBTHREADS) ? trace->header->header_nb_threads :
                        NBTHREADS;
        trace->header_buffer_ptr = (litl_buffer_t) realloc(
                trace->header_buffer_ptr,
                sizeof(litl_general_header_t)
                        + (nb_threads + 1) * sizeof(litl_thread_pairs_t));

        lseek(arch->f_handle, trace->triples->offset, SEEK_SET);
        trace->header_size = sizeof(litl_general_header_t)
                + (trace->header->header_nb_threads + 1)
                        * sizeof(litl_thread_pairs_t);
        int res = read(arch->f_handle, trace->header_buffer_ptr,
                trace->header_size);
        if (res == -1) {
            perror("Could not read the trace header!");
            exit(EXIT_FAILURE);
        }
        trace->header = (litl_general_header_t *) trace->header_buffer_ptr;
    }

    trace->header_buffer = trace->header_buffer_ptr
            + sizeof(litl_general_header_t);
}

/*
 * Reads another portion of pairs(tid, offset) from the trace file
 */
static void __litl_read_next_pairs_buffer(litl_read_trace_t* arch,
        litl_read_process_events_t* trace, litl_offset_t offset) {

    lseek(arch->f_handle, offset, SEEK_SET);

    litl_med_size_t nb_threads =
            (trace->nb_buffers - trace->header->header_nb_threads) > NBTHREADS ? NBTHREADS :
                    (trace->nb_buffers - trace->header->header_nb_threads);

    int res = read(arch->f_handle,
            trace->header_buffer_ptr + sizeof(litl_general_header_t),
            (nb_threads + 1) * sizeof(litl_thread_pairs_t));
    trace->header_buffer = trace->header_buffer_ptr
            + sizeof(litl_general_header_t);

    if (res == -1) {
        perror(
                "Could not read the next part of pairs (tid, offset) from the trace file!");
        exit(EXIT_FAILURE);
    }
}

/*
 * Initializes buffer of each recorded thread. One buffer per thread.
 */
static void __litl_read_init_buffers(litl_read_trace_t* arch,
        litl_read_process_events_t* trace) {
    litl_med_size_t i, size;
    litl_thread_pairs_t *tids;

    size = sizeof(litl_thread_pairs_t);
    // init nb_buffers and allocate memory
    trace->nb_buffers = trace->header->nb_threads;
    trace->buffers = (litl_read_thread_events_t*) malloc(
            trace->nb_buffers * sizeof(litl_read_thread_events_t));

    // increase a bit the buffer size 'cause of the possible event's tail and
    //   the offset
    trace->header->buffer_size += __litl_get_reg_event_size(LITL_MAX_PARAMS)
            + __litl_get_reg_event_size(0);

    for (i = 0; i < trace->nb_buffers; i++) {
        /* read pairs (tid, offset) */
        tids = (litl_thread_pairs_t *) trace->header_buffer;

        // deal with slots of pairs
        if ((tids->tid == 0) && (tids->offset != 0)) {
            __litl_read_next_pairs_buffer(arch, trace,
                    trace->triples->offset + tids->offset);
            tids = (litl_thread_pairs_t *) trace->header_buffer;
        }

        // end of reading pairs
        if ((tids->tid == 0) && (tids->offset == 0))
            break;

        trace->buffers[i].tids = (litl_thread_pairs_t*) malloc(
                sizeof(litl_thread_pairs_t));
        trace->buffers[i].tids->tid = tids->tid;
        trace->buffers[i].tids->offset = tids->offset;

        trace->header_buffer += size;

        /* read chunks of data */
        trace->buffers[i].buffer_ptr = (litl_buffer_t) malloc(
                trace->header->buffer_size);

        // use offsets in order to access a chuck of data that corresponds to
        //   each thread
        lseek(arch->f_handle,
                trace->triples->offset + trace->buffers[i].tids->offset,
                SEEK_SET);
        int res = read(arch->f_handle, trace->buffers[i].buffer_ptr,
                trace->header->buffer_size);

        // If the end of file is reached, then all data are read. So, res is 0.
        // Otherwise, res is either an error or the number of elements, which is 1.
        if (res == -1) {
            perror(
                    "Could not read the first partition of data from the trace file!");
            exit(EXIT_FAILURE);
        }

        trace->buffers[i].buffer = trace->buffers[i].buffer_ptr;
        trace->buffers[i].tracker = trace->header->buffer_size;
        trace->buffers[i].offset = 0;
    }
}

/*
 * Opens an archive of traces
 */
litl_read_trace_t *litl_read_open_trace(const char* filename) {
    litl_read_trace_t *arch = malloc(sizeof(litl_read_trace_t));

    // open a trace file
    if ((arch->f_handle = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // init the archive header
    __litl_read_init_header(arch);

    return arch;
}

/*
 * Initializes the archive's traces
 */
void litl_read_init_processes(litl_read_trace_t* arch) {

    arch->processes = (litl_read_process_events_t *) malloc(
            arch->nb_processes * sizeof(litl_read_process_events_t));

    if (arch->header->is_trace_archive) {
        // archive of traces
        litl_med_size_t i, size;
        size = sizeof(litl_process_triples_t);

        for (i = 0; i < arch->nb_processes; i++) {
            // TODO: implement this loop as recursion in order to deal with
            //       archives of archives
            // read triples that contain offset from the beginning of the archive
            arch->processes[i].triples =
                    (litl_process_triples_t *) arch->header_buffer;

            arch->header_buffer += size;

            arch->processes[i].cur_index = -1;
            arch->processes[i].is_initialized = 0;

            // init the trace header
            __litl_read_init_trace_header(arch, &arch->processes[i]);

            // init buffers of events: one buffer per thread
            __litl_read_init_buffers(arch, &arch->processes[i]);
        }
    } else {
        // regular trace
        arch->processes->triples = (litl_process_triples_t *) malloc(
                sizeof(litl_process_triples_t));
        arch->processes->triples->offset = 0;

        arch->processes->cur_index = -1;
        arch->processes->is_initialized = 0;

        // init the trace header
        __litl_read_init_trace_header(arch, arch->processes);

        // init buffers of events: one buffer per thread
        __litl_read_init_buffers(arch, arch->processes);
    }
}

/*
 * Returns a pointer to the trace header
 */
litl_general_header_t* litl_read_get_trace_header(litl_read_trace_t* arch) {
    return arch->processes[0].header;
}

/*
 * Sets the buffer size
 */
void litl_read_set_buffer_size(litl_read_trace_t* arch,
        const litl_size_t buf_size) {
    litl_med_size_t i;

    for (i = 0; i < arch->nb_processes; i++)
        arch->processes[i].header->buffer_size = buf_size;
}

/*
 * Returns the buffer size
 */
litl_size_t litl_get_buffer_size(litl_read_trace_t* arch) {
    return arch->processes[0].header->buffer_size;
}

/*
 * Reads another portion of data from the trace file to the buffer
 */
static void __litl_read_next_buffer(litl_read_trace_t* arch,
        litl_med_size_t trace_index, litl_med_size_t buffer_index) {
    litl_read_process_events_t* trace = &arch->processes[trace_index];

    lseek(arch->f_handle,
            trace->triples->offset + trace->buffers[buffer_index].tids->offset,
            SEEK_SET);
    trace->buffers[buffer_index].offset = 0;

    int res = read(arch->f_handle, trace->buffers[buffer_index].buffer_ptr,
            trace->header->buffer_size);

    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if (res == -1) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    trace->buffers[buffer_index].buffer =
            trace->buffers[buffer_index].buffer_ptr;
    trace->buffers[buffer_index].tracker = trace->buffers[buffer_index].offset
            + trace->header->buffer_size;
}

/*
 * Resets the trace buffer
 */
void litl_read_reset_trace(litl_read_trace_t* arch, litl_med_size_t trace_index) {
    litl_med_size_t i;
    litl_read_process_events_t* trace = &arch->processes[trace_index];

    for (i = 0; i < trace->nb_buffers; i++)
        trace->buffers[i].buffer = trace->buffers[i].buffer_ptr;
}

/*
 * Reads an event
 */
static litl_read_event_t* __litl_read_event(litl_read_trace_t* arch,
        litl_med_size_t trace_index, litl_med_size_t buffer_index) {
    uint8_t to_be_loaded;
    litl_t* event;
    litl_buffer_t* buffer;
    litl_read_process_events_t* trace = &arch->processes[trace_index];

    buffer = (litl_buffer_t*) trace->buffers[buffer_index].buffer;
    to_be_loaded = 0;

    if (!buffer) {
        trace->buffers[buffer_index].cur_event.event = NULL;
        return NULL;
    }

    event = (litl_t *) buffer;

    // While reading events from the buffer, there can be two situations:
    // 1. The situation when the buffer contains exact number of events;
    // 2. The situation when only a part of the last event is loaded.
    // Check whether the main four components (tid, time, code, nb_params) are
    //   loaded.
    // Check whether all arguments are loaded.
    // If any of these cases is not true, the next part of the trace plus
    // the current event is loaded to the buffer
    litl_size_t remaining_size = trace->buffers[buffer_index].tracker
            - trace->buffers[buffer_index].offset;
    if (remaining_size < __litl_get_reg_event_size(0)) {
        // this event is truncated. We can't even read the nb_param field
        to_be_loaded = 1;
    } else {
        // The nb_param (or size) field is available. Let's check whether
        //   the event is truncated
        litl_med_size_t event_size = __litl_get_gen_event_size(event);
        if (remaining_size < event_size)
            to_be_loaded = 1;
    }

    // event that stores tid and offset
    if (event->code == LITL_OFFSET_CODE) {
        if (event->parameters.offset.offset != 0) {
            trace->buffers[buffer_index].tids->offset =
                    event->parameters.offset.offset;
            to_be_loaded = 1;
        } else {
            trace->buffers[buffer_index].buffer = NULL;
            *buffer = NULL;
            trace->buffers[buffer_index].cur_event.event = NULL;
            return NULL;
        }
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __litl_read_next_buffer(arch, trace_index, buffer_index);
        buffer = &trace->buffers[buffer_index].buffer;
        event = (litl_t *) *buffer;
    }

    // move pointer to the next event and update __offset
    litl_med_size_t evt_size = __litl_get_gen_event_size(event);
    trace->buffers[buffer_index].buffer += evt_size;
    trace->buffers[buffer_index].offset += evt_size;

    trace->buffers[buffer_index].cur_event.event = event;
    trace->buffers[buffer_index].cur_event.tid =
            trace->buffers[buffer_index].tids->tid;
    return &trace->buffers[buffer_index].cur_event;
}

/*
 * Searches for the next event inside the trace
 */
litl_read_event_t* litl_read_next_trace_event(litl_read_trace_t* arch,
        litl_med_size_t trace_index) {
    litl_med_size_t i;
    litl_time_t min_time = -1;
    litl_read_process_events_t* trace = &arch->processes[trace_index];

    if (!trace->is_initialized) {
        for (i = 0; i < trace->nb_buffers; i++)
            __litl_read_event(arch, trace_index, i);

        trace->cur_index = -1;
        trace->is_initialized = 1;
    }

    // read the next event from the buffer
    if (trace->cur_index != -1)
        __litl_read_event(arch, trace_index, trace->cur_index);

    int found = 0;
    for (i = 0; i < trace->nb_buffers; i++) {
        litl_read_event_t *evt = LITL_READ_GET_CUR_EVENT_PER_THREAD(trace, i);
        if ( evt && evt->event && (LITL_READ_GET_TIME(evt) < min_time)) {
            found = 1;
            min_time = LITL_READ_GET_TIME(evt);
            trace->cur_index = i;
        }
    }

    if (found)
        return LITL_READ_GET_CUR_EVENT(trace);

    return NULL;
}

/*
 * Reads the next event either from a regular trace or an archive
 */
litl_read_event_t* litl_read_next_event(litl_read_trace_t* arch) {
    litl_med_size_t i;
    litl_read_event_t* event = NULL;

    for (i = 0; i < arch->nb_processes; i++) {
        event = litl_read_next_trace_event(arch, i);

        if (event != NULL)
            break;
    }

    return event;
}

/*
 * Closes the trace and frees the buffer
 */
void litl_read_finalize_trace(litl_read_trace_t* arch) {
    litl_med_size_t i, j;

    // close the file
    close(arch->f_handle);
    arch->f_handle = -1;

    // free traces
    for (i = 0; i < arch->nb_processes; i++) {
        free(arch->processes[i].header_buffer_ptr);

        for (j = 0; j < arch->processes[i].nb_buffers; j++) {
            free(arch->processes[i].buffers[j].buffer_ptr);
            free(arch->processes[i].buffers[j].tids);
        }

        free(arch->processes[i].buffers);
    }

    // free an archive
    free(arch->processes);
    free(arch->header_buffer_ptr);
    free(arch);

    // set pointers to NULL
    arch->header_buffer_ptr = NULL;
    arch->processes = NULL;
    arch = NULL;
}
