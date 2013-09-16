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
 * Initializes the trace header
 */
static void __litl_read_init_trace_header(litl_read_trace_t* trace) {
    int res;

    litl_size_t header_size, general_header_size;
    general_header_size = sizeof(litl_general_header_t);

    // read the trace header
    header_size = sizeof(litl_general_header_t);
    trace->header_buffer_ptr = (litl_buffer_t) malloc(header_size);
    if (!trace->header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    res = read(trace->f_handle, trace->header_buffer_ptr, header_size);
    // If the end of file is reached, then all data are read; res = 0.
    //   Otherwise, res equals the number of elements (= 1) or the error
    //   occurred and res = -1.
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }

    // init the trace header
    trace->header = (litl_general_header_t *) trace->header_buffer_ptr;

    // get the number of processes
    trace->nb_processes = trace->header->nb_processes;

    // relocate the header buffer
    header_size += trace->nb_processes * sizeof(litl_process_header_t);
    trace->header_buffer_ptr = (litl_buffer_t) realloc(trace->header_buffer_ptr,
            header_size);

    // read the trace header
    res = read(trace->f_handle, trace->header_buffer_ptr + general_header_size,
            header_size - general_header_size);
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header = (litl_general_header_t *) trace->header_buffer_ptr;
    trace->header_buffer = trace->header_buffer_ptr + general_header_size;
}

/*
 * Initializes the trace header, meaning it reads chunks with all pairs
 */
static void __litl_read_init_process_header(litl_read_trace_t* trace,
        litl_read_process_t* process) {

    // init the header structure
    litl_trace_size_t header_size;
    litl_med_size_t nb_threads =
            (process->header->header_nb_threads > NBTHREADS) ? process->header->header_nb_threads :
                    NBTHREADS;
    header_size = (nb_threads + 1) * sizeof(litl_thread_pair_t);
    process->header_buffer_ptr = (litl_buffer_t) malloc(header_size);

    // read threads pairs (tid, offset)
    lseek(trace->f_handle, process->header->offset, SEEK_SET);
    header_size = (process->header->header_nb_threads + 1)
            * sizeof(litl_thread_pair_t);
    int res = read(trace->f_handle, process->header_buffer_ptr, header_size);
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    process->header_buffer = process->header_buffer_ptr;
}

/*
 * Reads another portion of pairs(tid, offset) from the trace file
 */
static void __litl_read_next_pairs_buffer(litl_read_trace_t* trace,
        litl_read_process_t* process, litl_offset_t offset) {

    lseek(trace->f_handle, offset, SEEK_SET);

    litl_med_size_t nb_threads =
            (process->nb_threads - process->header->header_nb_threads)
                    > NBTHREADS ? NBTHREADS :
                    (process->nb_threads - process->header->header_nb_threads);

    int res = read(trace->f_handle, process->header_buffer_ptr,
            (nb_threads + 1) * sizeof(litl_thread_pair_t));
    process->header_buffer = process->header_buffer_ptr;

    if (res == -1) {
        perror(
                "Could not read the next part of pairs (tid, offset) from the trace file!");
        exit(EXIT_FAILURE);
    }
}

/*
 * Initializes buffers -- one buffer per thread.
 */
static void __litl_read_init_threads(litl_read_trace_t* trace,
        litl_read_process_t* process) {
    litl_med_size_t thread_index, size;
    litl_thread_pair_t *thread_pair;

    size = sizeof(litl_thread_pair_t);
    // init nb_threads and allocate memory
    process->nb_threads = process->header->nb_threads;
    process->threads = (litl_read_thread_t **) malloc(
            process->nb_threads * sizeof(litl_read_thread_t*));

    // increase a bit the buffer size 'cause of the event's tail and the offset
    process->header->buffer_size += __litl_get_reg_event_size(LITL_MAX_PARAMS)
            + __litl_get_reg_event_size(0);

    for (thread_index = 0; thread_index < process->nb_threads; thread_index++) {
        // allocate thread structure
        process->threads[thread_index] = (litl_read_thread_t *) malloc(
                sizeof(litl_read_thread_t));
        process->threads[thread_index]->thread_pair =
                (litl_thread_pair_t *) malloc(sizeof(litl_thread_pair_t));
        process->threads[thread_index]->buffer_ptr = (litl_buffer_t) malloc(
                process->header->buffer_size);

        // read pairs (tid, offset)
        thread_pair = (litl_thread_pair_t *) process->header_buffer;

        // deal with slots of pairs
        if ((thread_pair->tid == 0) && (thread_pair->offset != 0)) {
            __litl_read_next_pairs_buffer(trace, process,
                    process->header->offset + thread_pair->offset);
            thread_pair = (litl_thread_pair_t *) process->header_buffer;
        }

        // end of reading pairs
        if ((thread_pair->tid == 0) && (thread_pair->offset == 0))
            break;

        process->threads[thread_index]->thread_pair->tid = thread_pair->tid;
        // use two offsets: process and thread. Process offset for a position
        //  of thread pairs; thread offset for a position of events
        process->threads[thread_index]->thread_pair->offset =
                thread_pair->offset + process->header->offset;

        // read chunks of data
        // use offsets in order to access a chuck of data that corresponds to
        //   each thread
        lseek(trace->f_handle,
                process->threads[thread_index]->thread_pair->offset, SEEK_SET);
        int res = read(trace->f_handle,
                process->threads[thread_index]->buffer_ptr,
                process->header->buffer_size);
        if (res == -1) {
            perror(
                    "Could not read the first partition of data from the trace file!");
            exit(EXIT_FAILURE);
        }

        process->threads[thread_index]->buffer =
                process->threads[thread_index]->buffer_ptr;
        process->threads[thread_index]->tracker = process->header->buffer_size;
        process->threads[thread_index]->offset = 0;

        process->header_buffer += size;
    }
}

/*
 * Opens a trace
 */
litl_read_trace_t *litl_read_open_trace(const char* filename) {
    litl_read_trace_t *trace = (litl_read_trace_t *) malloc(
            sizeof(litl_read_trace_t));

    // open a trace file
    if ((trace->f_handle = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // init the trace header
    __litl_read_init_trace_header(trace);

    return trace;
}

/*
 * Initializes processes as trace may store multiple processes
 */
void litl_read_init_processes(litl_read_trace_t* trace) {

    trace->processes = (litl_read_process_t **) malloc(
            trace->nb_processes * sizeof(litl_read_process_t*));

    litl_med_size_t process_index, size;
    size = sizeof(litl_process_header_t);

    for (process_index = 0; process_index < trace->nb_processes;
            process_index++) {
        trace->processes[process_index] = (litl_read_process_t *) malloc(
                sizeof(litl_read_process_t));

        // read the process header
        trace->processes[process_index]->header =
                (litl_process_header_t *) trace->header_buffer;
        trace->header_buffer += size;

        trace->processes[process_index]->cur_index = -1;
        trace->processes[process_index]->is_initialized = 0;

        // init the process header
        __litl_read_init_process_header(trace, trace->processes[process_index]);

        // init buffers of events: one buffer per thread
        __litl_read_init_threads(trace, trace->processes[process_index]);
    }
}

/*
 * Returns a pointer to the trace header
 */
litl_general_header_t* litl_read_get_trace_header(litl_read_trace_t* trace) {
    return trace->header;
}

/*
 * Returns a pointer to the process header
 */
litl_process_header_t* litl_read_get_process_header(
        litl_read_process_t* process) {
    return process->header;
}

/*
 * Sets the buffer size
 */
void litl_read_set_buffer_size(litl_read_trace_t* trace,
        const litl_size_t buf_size) {
    litl_med_size_t i;

    for (i = 0; i < trace->nb_processes; i++)
        trace->processes[i]->header->buffer_size = buf_size;
}

/*
 * Returns the buffer size
 */
litl_size_t litl_read_get_buffer_size(litl_read_trace_t* trace) {
    return trace->processes[0]->header->buffer_size;
}

/*
 * Reads a next portion of events from the trace file to the buffer
 */
static void __litl_read_next_buffer(litl_read_trace_t* trace,
        litl_read_process_t* process, litl_med_size_t thread_index) {

    lseek(trace->f_handle,
            process->header->offset
                    + process->threads[thread_index]->thread_pair->offset,
            SEEK_SET);

    process->threads[thread_index]->offset = 0;

    // read portion of next events
    int res = read(trace->f_handle, process->threads[thread_index]->buffer_ptr,
            process->header->buffer_size);
    if (res == -1) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    process->threads[thread_index]->buffer =
            process->threads[thread_index]->buffer_ptr;
    process->threads[thread_index]->tracker =
            process->threads[thread_index]->offset
                    + process->header->buffer_size;
}

/*
 * Resets the thread buffers of a given process
 */
void litl_read_reset_process(litl_read_process_t* process) {
    litl_med_size_t thread_index;

    for (thread_index = 0; thread_index < process->nb_threads; thread_index++)
        process->threads[thread_index]->buffer =
                process->threads[thread_index]->buffer_ptr;
}

/*
 * Reads an event
 */
static litl_read_event_t* __litl_read_next_thread_event(
        litl_read_trace_t* trace, litl_read_process_t* process,
        litl_med_size_t thread_index) {

    litl_data_t to_be_loaded;
    litl_t* event;
    litl_buffer_t buffer;

    buffer = process->threads[thread_index]->buffer;
    to_be_loaded = 0;

    if (!buffer) {
        process->threads[thread_index]->cur_event.event = NULL;
        return NULL ;
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
    litl_size_t remaining_size = process->threads[thread_index]->tracker
            - process->threads[thread_index]->offset;
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
            process->threads[thread_index]->thread_pair->offset =
                    event->parameters.offset.offset;
            to_be_loaded = 1;
        } else {
            buffer = NULL;
            process->threads[thread_index]->cur_event.event = NULL;
            return NULL ;
        }
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __litl_read_next_buffer(trace, process, thread_index);
        buffer = process->threads[thread_index]->buffer;
        event = (litl_t *) buffer;
    }

    // move pointer to the next event and update __offset
    litl_med_size_t evt_size = __litl_get_gen_event_size(event);
    process->threads[thread_index]->buffer += evt_size;
    process->threads[thread_index]->offset += evt_size;

    process->threads[thread_index]->cur_event.event = event;
    process->threads[thread_index]->cur_event.tid =
            process->threads[thread_index]->thread_pair->tid;

    return &process->threads[thread_index]->cur_event;
}

/*
 * Searches for the next event inside the trace
 */
litl_read_event_t* litl_read_next_process_event(litl_read_trace_t* trace,
        litl_read_process_t* process) {

    litl_med_size_t thread_index;
    litl_time_t min_time = -1;

    if (!process->is_initialized) {
        for (thread_index = 0; thread_index < process->nb_threads;
                thread_index++)
            __litl_read_next_thread_event(trace, process, thread_index);

        process->cur_index = -1;
        process->is_initialized = 1;
    }

    // read the next event from the buffer
    if (process->cur_index != -1)
        __litl_read_next_thread_event(trace, process, process->cur_index);

    int found = 0;
    for (thread_index = 0; thread_index < process->nb_threads; thread_index++) {
        litl_read_event_t *evt =
                LITL_READ_GET_CUR_EVENT_PER_THREAD(process, thread_index);
                if ( evt && evt->event && (LITL_READ_GET_TIME(evt) < min_time)) {
                    found = 1;
                    min_time = LITL_READ_GET_TIME(evt);
                    process->cur_index = thread_index;
                }
            }

    if (found)
        return LITL_READ_GET_CUR_EVENT(process);

    return NULL ;
}

/*
 * Reads the next event from a trace
 */
litl_read_event_t* litl_read_next_event(litl_read_trace_t* trace) {
    litl_med_size_t process_index;
    litl_read_event_t* event = NULL;

    for (process_index = 0; process_index < trace->nb_processes;
            process_index++) {
        event = litl_read_next_process_event(trace,
                trace->processes[process_index]);

        if (event != NULL )
            break;
    }

    return event;
}

/*
 * Closes the trace and frees the buffer
 */
void litl_read_finalize_trace(litl_read_trace_t* trace) {
    litl_med_size_t process_index, thread_index;

    // close the file
    close(trace->f_handle);
    trace->f_handle = -1;

    // free traces
    for (process_index = 0; process_index < trace->nb_processes;
            process_index++) {

        for (thread_index = 0;
                thread_index < trace->processes[process_index]->nb_threads;
                thread_index++) {
            free(
                    trace->processes[process_index]->threads[thread_index]->thread_pair);
            free(
                    trace->processes[process_index]->threads[thread_index]->buffer_ptr);
            free(trace->processes[process_index]->threads[thread_index]);
        }

        free(trace->processes[process_index]->threads);
        free(trace->processes[process_index]->header_buffer_ptr);
        free(trace->processes[process_index]);
    }

    // free a trace structure
    free(trace->processes);
    free(trace->header_buffer_ptr);
    free(trace);

    // set the trace pointer to NULL
    trace = NULL;
}
