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

#include "litl_macro.h"
#include "litl_read.h"

static char* __input_filename = "trace";

static void __usage(int argc __attribute__((unused)), char **argv) {
    fprintf(stderr, "Usage: %s [-f input_filename] \n", argv[0]);
    printf("       -?, -h:    Display this help and exit\n");
}

static void __parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-f") == 0)) {
            __input_filename = argv[++i];
        } else if ((strcmp(argv[i], "-h") || strcmp(argv[i], "-?")) == 0) {
            __usage(argc, argv);
            exit(-1);
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            __usage(argc, argv);
            exit(-1);
        }
    }

    if (strcmp(__input_filename, "trace") == 0) {
        __usage(argc, argv);
        exit(-1);
    }
}

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
litl_trace_read_t *litl_open_trace(const char* filename) {
    litl_trace_read_t *trace = malloc(sizeof(litl_trace_read_t));

    // open a trace file
    if ((trace->f_handle = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }

    trace->cur_index = -1;
    trace->initialized = 0;

    // init the header structure
    trace->header_size = 1536; // 1.5KB
    trace->header_buffer_ptr = (litl_buffer_t) malloc(trace->header_size);
    if (!trace->header_buffer_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }

    // read the header
    int res = read(trace->f_handle, trace->header_buffer_ptr, trace->header_size);

    // If the end of file is reached, then all data are read. So, res is 0.
    //      Otherwise, res is either an error or the number of elements, which is 1.
    if (res == -1) {
        perror("Could not read the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header_buffer = trace->header_buffer_ptr;

    litl_init_trace_header(trace);

    // increase a bit the buffer size 'cause of the possible event's tail and the offset
    trace->buffer_size = trace->header->buffer_size + get_event_size(LITL_MAX_PARAMS) + get_event_size(0);
    litl_init_buffers(trace);

    return trace;
}

/*
 * This function sets the buffer size
 */
void litl_set_buffer_size(litl_trace_read_t* trace, const litl_size_t buf_size) {
    trace->buffer_size = buf_size;
}

/*
 * This function returns the buffer size
 */
litl_size_t litl_get_buffer_size(litl_trace_read_t* trace) {
    return trace->buffer_size;
}

/*
 * This function initializes the trace header
 */
void litl_init_trace_header(litl_trace_read_t* trace) {
    litl_size_t i, size;

    trace->header = (litl_header_t *) trace->header_buffer;
    size = sizeof(litl_header_t);
    trace->header_buffer += size;

    // init nb_buffers
    trace->nb_buffers = trace->header->nb_threads;

    trace->buffers = (litl_trace_read_thread_t*) malloc(trace->nb_buffers * sizeof(litl_trace_read_thread_t));
    size = sizeof(litl_header_tids_t);

    for (i = 0; i < trace->nb_buffers; i++) {
        trace->buffers[i].tids = (litl_header_tids_t *) trace->header_buffer;
        trace->header_buffer += size;
    }
}

/*
 * This function return a pointer to the trace header
 */
litl_header_t* litl_get_trace_header(litl_trace_read_t* trace) {
    return trace->header;
}

/*
 * This function initializes buffer of each recorded thread. A buffer per thread.
 */
void litl_init_buffers(litl_trace_read_t* trace) {
    litl_size_t i;

    for (i = 0; i < trace->nb_buffers; i++) {
        // use offsets in order to access a chuck of data that corresponds to each thread
        lseek(trace->f_handle, trace->buffers[i].tids->offset, SEEK_SET);

        trace->buffers[i].buffer_size = trace->buffer_size;
        trace->buffers[i].buffer_ptr = (litl_buffer_t) malloc(trace->buffer_size);

        int res = read(trace->f_handle, trace->buffers[i].buffer_ptr, trace->buffer_size);

        // If the end of file is reached, then all data are read. So, res is 0.
        // Otherwise, res is either an error or the number of elements, which is 1.
        if (res == -1) {
            perror("Could not read the first partition of data from the trace file!");
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
static void __next_trace(litl_trace_read_t* trace, litl_size_t index) {
    lseek(trace->f_handle, trace->buffers[index].tids->offset, SEEK_SET);
    trace->buffers[index].offset = 0;

    int res = read(trace->f_handle, trace->buffers[index].buffer_ptr, trace->buffer_size);

    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if (res == -1) {
        perror("Could not read the next part of the trace file!");
        exit(EXIT_FAILURE);
    }

    trace->buffers[index].buffer = trace->buffers[index].buffer_ptr;
    trace->buffers[index].tracker = trace->buffers[index].offset + trace->buffer_size;
}

/*
 * This function resets the trace
 */
void litl_reset_trace(litl_trace_read_t* trace, litl_size_t index) {
    trace->buffers[index].buffer = trace->buffers[index].buffer_ptr;
}

/*
 * This function reads an event
 */
litl_read_t* litl_read_event(litl_trace_read_t* trace, litl_size_t index) {
    uint8_t to_be_loaded;
    litl_size_t size;
    litl_t* event;
    litl_buffer_t* buffer;
    buffer = trace->buffers[index].buffer;
    to_be_loaded = 0;

    if (!buffer) {
        trace->buffers[index].cur_event.event = NULL;
        return NULL ;
    }

    event = (litl_t *) buffer;

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
        litl_size_t event_size = get_event_size_type(event);
        if (remaining_size < event_size)
            to_be_loaded = 1;
    }

    // event that stores tid and offset
    if (event->code == LITL_OFFSET_CODE) {
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
        event = (litl_t *) *buffer;
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
litl_read_t* litl_next_buffer_event(litl_trace_read_t* trace, litl_size_t index) {
    return litl_read_event(trace, index);
}

/*
 * This function searches for the next event inside the trace
 */
litl_read_t* litl_next_trace_event(litl_trace_read_t* trace) {
    litl_size_t i;
    litl_time_t min_time = -1;

    if (!trace->initialized) {
        for (i = 0; i < trace->nb_buffers; i++)
            litl_next_buffer_event(trace, i);

        trace->cur_index = -1;
        trace->initialized = 1;
    }

    // read the next event from the buffer
    if (trace->cur_index != -1)
        litl_next_buffer_event(trace, trace->cur_index);

    int found = 0;
    for (i = 0; i < trace->nb_buffers; i++) {
        litl_read_t *evt = GET_CUR_EVENT_PER_THREAD(trace, i);
        if ( evt && evt->event && (LITL_GET_TIME(evt) < min_time)) {
            found = 1;
            min_time = LITL_GET_TIME(evt);
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
void litl_close_trace(litl_trace_read_t* trace) {
    litl_size_t i;

    // close the file
    close(trace->f_handle);

    // free buffers
    free(trace->header_buffer_ptr);
    for (i = 0; i < trace->nb_buffers; i++) {
        free(trace->buffers[i].buffer_ptr);
    }
    free(trace->buffers);

    // set pointers to NULL
    trace->header_buffer_ptr = NULL;
    trace->buffers = NULL;
    trace->f_handle = -1;
}

int main(int argc, char **argv) {
    litl_size_t i;
    litl_read_t* event;
    litl_trace_read_t *trace;
    litl_header_t* header;

    // parse the arguments passed to this program
    __parse_args(argc, argv);

    trace = litl_open_trace(__input_filename);
    header = litl_get_trace_header(trace);

    // print the header
    printf(" LiTL v.%s\n", header->liblitl_ver);
    printf(" %s\n", header->sysinfo);
    printf(" nb_threads \t %d\n", header->nb_threads);
    printf(" buffer_size \t %d\n", header->buffer_size);

    while (1) {
        event = litl_next_trace_event(trace);

        if (event == NULL )
            break;

        switch (LITL_GET_TYPE(event)) {
        case LITL_TYPE_REGULAR: { // regular event
            printf("%"PRTIu64" \t  Reg   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_GET_TIME(event), LITL_REGULAR(event)->nb_params);

            for (i = 0; i < LITL_REGULAR(event)->nb_params; i++)
                printf("\t %"PRTIx64, LITL_REGULAR(event)->param[i]);
            break;
        }
        case LITL_TYPE_RAW: { // raw event
            LITL_GET_CODE(event) = clear_bit(LITL_GET_CODE(event));
            printf("%"PRTIu64" \t  Raw   %"PRTIx32" \t %"PRTIu64" \t %"PRTIu32, LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_GET_TIME(event), LITL_RAW(event)->size);
            printf("\t %s", (litl_data_t *) LITL_RAW(event)->data);
            break;
        }
        case LITL_TYPE_PACKED: { // packed event
            printf("%"PRTIu64" \t  Packed   %"PRTIx32" \t %"PRTIu64"   %"PRTIu32"\t", LITL_GET_TID(event),
                    LITL_GET_CODE(event), LITL_GET_TIME(event), LITL_PACKED(event)->size);
            for (i = 0; i < LITL_PACKED(event)->size; i++) {
                printf(" %x", LITL_PACKED(event)->param[i]);
            }
            break;
        }
        case LITL_TYPE_OFFSET: { // offset event
            continue;
        }
        default: {
            fprintf(stderr, "Unknown event type %d\n", LITL_GET_TYPE(event));
            abort();
        }
        }

        printf("\n");
    }

    litl_close_trace(trace);

    return EXIT_SUCCESS;
}
