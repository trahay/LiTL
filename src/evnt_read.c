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

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_trace_read_t *evnt_open_trace(const char* filename) {
    evnt_trace_read_t *trace = malloc(sizeof(evnt_trace_read_t));

    // open the trace file
    if (!(trace->fp = fopen(filename, "r"))) {
        perror("Could not open the trace file for reading!");
        exit(EXIT_FAILURE);
    }

    trace->cur_index = -1;
    trace->initialized = 0;

    // init the header structure
    trace->header_size = 1536; // 1.5KB
    trace->header_buffer_ptr = (evnt_buffer_t) malloc(trace->header_size);
    if (!trace->header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    // read the header
    int res = fread(trace->header_buffer_ptr, trace->header_size, 1, trace->fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    //      Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header_buffer = trace->header_buffer_ptr;

    evnt_init_trace_header(trace);

    // increase a bit the buffer size 'cause of the possible event's tail and the offset
    trace->buffer_size = trace->header->buffer_size + get_event_size(EVNT_MAX_PARAMS) + get_event_size(0);
    evnt_init_buffers(trace);

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
    trace->header_buffer += size;

    // init nb_buffers
    trace->nb_buffers = trace->header->nb_threads;

    trace->buffers = (evnt_trace_read_thread_t*) malloc(trace->nb_buffers * sizeof(evnt_trace_read_thread_t));
    size = sizeof(evnt_header_tids_t);

    for (i = 0; i < trace->nb_buffers; i++) {
        trace->buffers[i].tids = (evnt_header_tids_t *) trace->header_buffer;
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

    evnt_size_t i;
    for (i = 0; i < trace->nb_buffers; i++) {
        // use offsets in order to access a chuck of data that corresponds to each thread
        fseek(trace->fp, trace->buffers[i].tids->offset, SEEK_SET);

        trace->buffers[i].buffer_size = trace->buffer_size;
        trace->buffers[i].buffer_ptr = (evnt_buffer_t) malloc(trace->buffer_size);

        int res = fread(trace->buffers[i].buffer_ptr, trace->buffer_size, 1, trace->fp);
        // If the end of file is reached, then all data are read. So, res is 0.
        // Otherwise, res is either an error or the number of elements, which is 1.
        if ((res != 0) && (res != 1)) {
            perror("Could read the first partition of data from the trace file!");
            exit(EXIT_FAILURE);
        }
        trace->buffers[i].buffer = trace->buffers[i].buffer_ptr;
        trace->buffers[i].tracker = trace->buffers[i].buffer_size;
        trace->buffers[i].offset = 0;
    }
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static void __next_trace(evnt_trace_read_t* trace, evnt_size_t index) {
    fseek(trace->fp, trace->buffers[index].tids->offset, SEEK_SET);
    trace->buffers[index].offset = 0;

    int res = fread(trace->buffers[index].buffer_ptr, trace->buffer_size, 1, trace->fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    trace->buffers[index].buffer = trace->buffers[index].buffer_ptr;
    trace->buffers[index].tracker = trace->buffers[index].offset + trace->buffer_size;
}

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_trace_read_t* trace, evnt_size_t index) {
    trace->buffers[index].buffer = trace->buffers[index].buffer_ptr;
}

/*
 * This function reads an event
 */
evnt_read_t* evnt_read_event(evnt_trace_read_t* trace, evnt_size_t index) {
    uint8_t to_be_loaded;
    evnt_size_t size;
    evnt_t* event;
    evnt_buffer_t* buffer;
    buffer = trace->buffers[index].buffer;
    to_be_loaded = 0;

    if (!buffer) {
        trace->buffers[index].cur_event.event = NULL;
        return NULL ;
    }

    event = (evnt_t *) buffer;

    /* While reading events from the buffer, there can be two situations:
     1. The situation when the buffer contains exact number of events;
     2. The situation when only a part of the last event is loaded to the buffer.
     Check whether the main four components (tid, time, code, nb_params) are loaded.
     Check whether all arguments are loaded.
     If any of these cases is not true, the next part of the trace plus the current event is loaded to the buffer.*/
    unsigned remaining_size = trace->buffers[index].tracker - trace->buffers[index].offset;
    if (remaining_size < get_event_size(0)) {
        /* this event is truncated. We can't even read the nb_param field */
        to_be_loaded = 1;
    } else {
        /* The nb_param (or size) field is available. Let's see if the event is truncated */
        evnt_size_t event_size = get_event_size_type(event);
        if (remaining_size < event_size)
            to_be_loaded = 1;
    }

    // event that stores tid and offset
    if (event->code == EVNT_OFFSET_CODE) {
        if (event->parameters.offset.offset != 0) {
            trace->buffers[index].tids->offset = event->parameters.offset.offset;
            to_be_loaded = 1;
        } else {
            trace->buffers[index].buffer = NULL;
            *buffer = NULL;
            trace->buffers[index].cur_event.event = NULL;
            return NULL ;
        }
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __next_trace(trace, index);
        buffer = &trace->buffers[index].buffer;
        event = (evnt_t *) *buffer;
    }

    // move pointer to the next event and update __offset
    unsigned evt_size = get_event_size_type(event);
    trace->buffers[index].buffer += evt_size;
    trace->buffers[index].offset += evt_size;

    trace->buffers[index].cur_event.event = event;
    trace->buffers[index].cur_event.tid = trace->buffers[index].tids->tid;
    return &trace->buffers[index].cur_event;
}

/*
 * This function searches for the next event inside each buffer
 */
evnt_read_t* evnt_next_buffer_event(evnt_trace_read_t* trace, evnt_size_t index) {
    return evnt_read_event(trace, index);
}

/*
 * This function searches for the next event inside the trace
 */
evnt_read_t* evnt_next_trace_event(evnt_trace_read_t* trace) {
    evnt_size_t i;
    evnt_time_t min_time = -1;

    if (!trace->initialized) {
        for (i = 0; i < trace->nb_buffers; i++)
            evnt_next_buffer_event(trace, i);

        trace->cur_index = -1;
        trace->initialized = 1;
    }

    // read the next event from the buffer
    if (trace->cur_index != -1)
        evnt_next_buffer_event(trace, trace->cur_index);

    int found = 0;
    for (i = 0; i < trace->nb_buffers; i++) {
        evnt_read_t *evt = GET_CUR_EVENT_PER_THREAD(trace, i);
        if ( evt && evt->event && (EVNT_GET_TIME(evt) < min_time)) {
            found = 1;
            min_time = EVNT_GET_TIME(evt);
            trace->cur_index = i;
        }
    }
    if (found)
        return GET_CUR_EVENT(trace);
    return NULL ;
}

/*
 * This function closes the trace and frees the buffer
 */
void evnt_close_trace(evnt_trace_read_t* trace) {
    evnt_size_t i;

    // close the file
    fclose(trace->fp);

    // free buffers
    free(trace->header_buffer_ptr);
    for (i = 0; i < trace->nb_buffers; i++) {
        free(trace->buffers[i].buffer_ptr);
    }
    free(trace->buffers);

    // set pointers to NULL
    trace->fp = NULL;
    trace->header_buffer_ptr = NULL;
    trace->buffers = NULL;
}

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_read_t* event;
    evnt_trace_read_t *trace;
    evnt_header_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        perror("Specify the name of the trace file using '-f'\n");
        exit(EXIT_FAILURE);
    }

    trace = evnt_open_trace(filename);
    header = evnt_get_trace_header(trace);

    // print the header
    printf(" libevnt v.%s\n", header->libevnt_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    while (1) {
        event = evnt_next_trace_event(trace);

        if (event == NULL )
            break;

        switch (EVNT_GET_TYPE(event)) {
        case EVNT_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, EVNT_GET_TID(event),
                    EVNT_GET_CODE(event), EVNT_GET_TIME(event), EVNT_REGULAR(event)->nb_params);

            for (i = 0; i < EVNT_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, EVNT_REGULAR(event)->param[i]);
            break;
        }
        case EVNT_TYPE_RAW: { // raw event
            EVNT_GET_CODE(event) = clear_bit(EVNT_GET_CODE(event));
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, EVNT_GET_TID(event),
                    EVNT_GET_CODE(event), EVNT_GET_TIME(event), EVNT_RAW(event)->size);
            printf("\t %s", (evnt_data_t *) EVNT_RAW(event)->data);
            break;
        }
        case EVNT_TYPE_PACKED: { // packed event
            printf("%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64"   %"PRTIu32"\t", EVNT_GET_TID(event),
                    EVNT_GET_CODE(event), EVNT_GET_TIME(event), EVNT_PACKED(event)->size);
            for (i = 0; i < EVNT_PACKED(event)->size; i++) {
                printf(" %x", EVNT_PACKED(event)->param[i]);
            }
            break;
        }
        case EVNT_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", EVNT_GET_TYPE(event));
            abort();
        }
        }

        printf("\n");
    }

    evnt_close_trace(trace);

    return EXIT_SUCCESS;
}
