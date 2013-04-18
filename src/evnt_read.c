/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include "evnt_macro.h"
#include "evnt_read.h"

// an array of events. Each one from a different thread
static evnt_t** arr_ev;
static int16_t trace_ind;
static uint8_t init_trace = 0;

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_trace_read_t evnt_open_trace(const char* filename) {
    evnt_trace_read_t trace;

    // open the trace file
    if (!(trace.fp = fopen(filename, "r"))) {
        perror("Could not open the trace file for reading!");
        exit(EXIT_FAILURE);
    }

    // init the header structure
    trace.header_size = 1536; // 1.5KB
    trace.header_buffer_ptr = (evnt_buffer_t) malloc(trace.header_size);
    if (!trace.header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    // read the header
    int res = fread(trace.header_buffer_ptr, trace.header_size, 1, trace.fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    //      Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace.header_buffer = trace.header_buffer_ptr;

    evnt_init_trace_header(&trace);

    // increase a bit the buffer size 'cause of the possible event's tail and the offset
    trace.buffer_size = trace.header->buffer_size + get_event_size(EVNT_MAX_PARAMS) + get_event_size(0);
    evnt_init_buffers(&trace);

    // allocate memory for a list of events;
    arr_ev = (evnt_t **) malloc(trace.buffer_size * sizeof(evnt_t *));

    return trace;
}

/*
 * This function sets the buffer size
 */
void evnt_set_buffer_size(evnt_trace_read_t* trace, const evnt_size_t buf_size) {
    trace->buffer_size = buf_size;
}

/*
 * This function returns the buffer size
 */
evnt_size_t evnt_get_buffer_size(evnt_trace_read_t* trace) {
    return trace->buffer_size;
}

/*
 * This function initializes the trace header
 */
void evnt_init_trace_header(evnt_trace_read_t* trace) {
    evnt_size_t i, size;

    trace->header = (evnt_header_t *) trace->header_buffer;
    size = sizeof(evnt_header_t);
    size = (evnt_size_t) ceil((double) size / sizeof(evnt_param_t));
    trace->header_buffer += size;

    // init nb_buffers
    trace->nb_buffers = trace->header->nb_threads;

    // set pairs (tid, offset)
    trace->tids = (evnt_header_tids_t **) malloc(trace->nb_buffers * sizeof(evnt_header_tids_t));
    size = (evnt_size_t) ceil((double) sizeof(evnt_header_tids_t) / sizeof(evnt_param_t));
    for (i = 0; i < trace->nb_buffers; i++) {
        trace->tids[i] = (evnt_header_tids_t *) trace->header_buffer;
        trace->header_buffer += size;
    }
}

/*
 * This function return a pointer to the trace header
 */
evnt_header_t* evnt_get_trace_header(evnt_trace_read_t* trace) {
    return trace->header;
}

/*
 * This function initializes buffer of each recorded thread. A buffer per thread.
 */
void evnt_init_buffers(evnt_trace_read_t* trace) {
    trace->buffer_ptr = (evnt_buffer_t *) malloc(trace->nb_buffers * sizeof(evnt_buffer_t));
    trace->buffer = (evnt_buffer_t *) malloc(trace->nb_buffers * sizeof(evnt_buffer_t));
    trace->offset = (evnt_offset_t *) malloc(trace->nb_buffers * sizeof(evnt_offset_t));
    trace->tracker = (evnt_offset_t *) malloc(trace->nb_buffers * sizeof(evnt_offset_t));

    evnt_size_t i;
    for (i = 0; i < trace->nb_buffers; i++) {
        // use offsets in order to access a chuck of data that corresponds to each thread
        fseek(trace->fp, trace->tids[i]->offset, SEEK_SET);

        trace->buffer_ptr[i] = (evnt_buffer_t) malloc(trace->buffer_size);
        int res = fread(trace->buffer_ptr[i], trace->buffer_size, 1, trace->fp);
        // If the end of file is reached, then all data are read. So, res is 0.
        // Otherwise, res is either an error or the number of elements, which is 1.
        if ((res != 0) && (res != 1)) {
            perror("Could read the first partition of data from the trace file!");
            exit(EXIT_FAILURE);
        }
        trace->buffer[i] = trace->buffer_ptr[i];
        trace->tracker[i] = trace->buffer_size;
        trace->offset[i] = 0;
    }
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static void __next_trace(evnt_trace_read_t* trace, evnt_size_t index) {
    fseek(trace->fp, trace->tids[index]->offset, SEEK_SET);
    trace->offset[index] = 0;

    int res = fread(trace->buffer_ptr[index], trace->buffer_size, 1, trace->fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    trace->buffer[index] = trace->buffer_ptr[index];
    trace->tracker[index] = trace->offset[index] + trace->buffer_size;
}

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_trace_read_t* trace, evnt_size_t index) {
    trace->buffer[index] = trace->buffer_ptr[index];
}

/*
 * This function reads an event
 */
evnt_t* evnt_read_event(evnt_trace_read_t* trace, evnt_size_t index) {
    uint8_t to_be_loaded;
    evnt_size_t size;
    evnt_t* event;
    evnt_buffer_t* buffer;
    buffer = &trace->buffer[index];
    to_be_loaded = 0;

    if (!*buffer)
        return NULL ;

    event = (evnt_t *) *buffer;

    /* While reading events from the buffer, there can be two situations:
     1. The situation when the buffer contains exact number of events;
     2. The situation when only a part of the last event is loaded to the buffer.
     Check whether the main four components (tid, time, code, nb_params) are loaded.
     Check whether all arguments are loaded.
     If any of these cases is not true, the next part of the trace plus the current event is loaded to the buffer.*/
    unsigned remaining_size = trace->tracker[index] - trace->offset[index];
    if (remaining_size < get_event_size(0)) {
        /* this event is truncated. We can't even read the nb_param field */
        to_be_loaded = 1;
    } else {
        /* The nb_param (or size) field is available. Let's see if the event is truncated */
        unsigned event_size = get_event_size_type(event);
        if (remaining_size < event_size)
            to_be_loaded = 1;
    }

    // event that stores tid and offset
    if (event->code == EVNT_OFFSET) {
        if (event->time != 0) {
            trace->tids[index]->offset = event->time;
            /**buffer += get_event_offset_components();
             -trace->offset += get_event_offset_size();*/
            to_be_loaded = 1;
        } else
            event->code = EVNT_TRACE_END;
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __next_trace(trace, index);
        buffer = &trace->buffer[index];
        event = (evnt_t *) *buffer;
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        trace->buffer[index] = NULL;
        *buffer = NULL;
        return NULL ;
    }

    // move pointer to the next event and update __offset
    unsigned evt_size = get_event_size_type(event);
    trace->buffer[index] = (evnt_buffer_t) (((uint8_t*) trace->buffer[index]) + evt_size);
    trace->offset[index] = (((uint8_t*) trace->offset[index]) + get_event_size_type(event));

    return event;
}

/*
 * This function searches for the next event inside each buffer
 */
evnt_t* evnt_next_buffer_event(evnt_trace_read_t* trace, evnt_size_t index) {
    return evnt_read_event(trace, index);
}

/*
 * This function searches for the next event inside the trace
 */
evnt_t* evnt_next_trace_event(evnt_trace_read_t* trace) {
    evnt_size_t i;
    evnt_time_t min_time = -1;

    if (!init_trace) {
        for (i = 0; i < trace->nb_buffers; i++)
            arr_ev[i] = evnt_next_buffer_event(trace, i);

        trace_ind = -1;
        init_trace = 1;
    }

    // read the next event from the buffer
    if (trace_ind != -1)
        arr_ev[trace_ind] = evnt_next_buffer_event(trace, trace_ind);

    for (i = 0; i < trace->nb_buffers; i++)
        if ((arr_ev[i]) && (arr_ev[i]->time < min_time)) {
            min_time = arr_ev[i]->time;
            trace_ind = i;
        }

    return arr_ev[trace_ind];
}

/*
 * This function closes the trace and frees the buffer
 */
void evnt_close_trace(evnt_trace_read_t* trace) {
    evnt_size_t i;

    free(arr_ev);
    // close the file
    fclose(trace->fp);

    // free buffers
    free(trace->header_buffer_ptr);
    for (i = 0; i < trace->nb_buffers; i++)
        free(trace->buffer_ptr[i]);
    free(trace->buffer_ptr);
    free(trace->buffer);
    free(trace->tids);
    free(trace->tracker);
    free(trace->offset);

    // set pointers to NULL
    trace->fp = NULL;
    trace->header_buffer_ptr = NULL;
    trace->buffer_ptr = NULL;
    trace->buffer = NULL;
    trace->tids = NULL;
    trace->tracker = NULL;
    trace->offset = NULL;
}

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_t* event;
    evnt_trace_read_t trace;
    evnt_header_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        perror("Specify the name of the trace file using '-f'\n");
        exit(EXIT_FAILURE);
    }

    trace = evnt_open_trace(filename);
    header = evnt_get_trace_header(&trace);

    // print the header
    printf(" libevnt v.%s\n", header->libevnt_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    while (1) {
        event = evnt_next_trace_event(&trace);

        if (event == NULL )
            break;

        switch (event->type) {
        case EVNT_TYPE_REGULAR: { // regular event
            printf("Reg\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, trace.tids[trace_ind]->tid,
                    event->time, event->parameters.regular.nb_params);

            for (i = 0; i < event->parameters.regular.nb_params; i++)
                printf("\t %"PRTIx64, event->parameters.regular.param[i]);
            break;
        }
        case EVNT_TYPE_RAW: { // raw event
            event->code = clear_bit(event->code);
            printf("Raw\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, trace.tids[trace_ind]->tid,
                    event->time, event->parameters.raw.size);
            printf("\t %s", (evnt_data_t *) event->parameters.raw.data);
            break;
        }
        case EVNT_TYPE_PACKED: { // packed event
            printf("Packed\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, trace.tids[trace_ind]->tid,
                    event->time, event->parameters.packed.size);
            for (i = 0; i < event->parameters.packed.size; i++) {
                printf("\t%x", event->parameters.packed.param[i]);
            }
            break;
        }
        case EVNT_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", event->type);
            *(int*) 0 = 0;
        }
        }

        printf("\n");
    }

    evnt_close_trace(&trace);

    return EXIT_SUCCESS;
}
