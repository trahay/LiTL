/*
 * evnt_read.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include "evnt_macro.h"
#include "evnt_read.h"

static FILE* __ftrace;
static evnt_trace_t __buffer_ptr;
static uint32_t __buffer_size = 256 * 1024; // 256KB is the optimal size on Intel Core 2
// offset from the beginning of the trace file
static uint32_t __offset = 0;
static uint32_t __tracker;

/*
 * This function initialize tracker using __offset and __buffer_size
 */
void __init_tracker() __attribute__((constructor));

void __init_tracker() {
    __tracker = __offset + __buffer_size;
}

/*
 * This function sets the buffer size
 */
void set_read_buffer_size(const uint32_t buf_size) {
    __offset = 0;
    __buffer_size = buf_size;
    __tracker = __offset + __buffer_size;
}

/*
 * This function returns the current trace, FILE pointer, and the current position in the file
 */
evnt_block_t get_evnt_block() {
    evnt_block_t block;

    block.fp = __ftrace;
    block.offset = __offset;
    block.trace = __buffer_ptr;

    return block;
}

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_trace_t open_trace(const char* filename) {
    struct stat st;

    if (!(__ftrace = fopen(filename, "r"))) {
        perror("Could not open the trace file for reading!");
        exit(EXIT_FAILURE);
    }

    if (stat(filename, &st) != 0)
        perror("Could not get the attributes of the trace file!");

    if (__buffer_size > st.st_size)
        __buffer_size = st.st_size;

    __buffer_ptr = malloc(__buffer_size);

    int res = fread(__buffer_ptr, __buffer_size, 1, __ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not copy the top of the trace file to a buffer!");
        exit(EXIT_FAILURE);
    }

    return __buffer_ptr;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static evnt_trace_t __next_trace() {
    fseek(__ftrace, __offset, SEEK_SET);

    int res = fread(__buffer_ptr, __buffer_size, 1, __ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not copy the next part of the trace file to a buffer!");
        exit(EXIT_FAILURE);
    }

    return __buffer_ptr;
}

/*
 * This function resets the trace
 */
void reset_trace(evnt_trace_t* buffer) {
    *buffer = __buffer_ptr;
}

/*
 * This function reads an event
 */
evnt_t* read_event(evnt_trace_t* buffer) {
    uint8_t to_be_loaded = 0;
    evnt_size_t size;
    evnt_t* event;

    if (*buffer == NULL )
        return NULL ;

    event = (evnt_t *) *buffer;

    /* While reading events from the buffer, there can be two situations:
     1. The situation when the buffer contains exact number of events;
     2. The situation when only a part of the last event is loaded to the buffer.
     Check whether the main four components (tid, time, code, nb_params) are loaded.
     Check whether all arguments are loaded.
     If any of these cases is not true, the next part of the trace plus the current event is loaded to the buffer.*/
    // regular event
    if ((__tracker - __offset <= sizeof(evnt_t))
            || ((get_bit(event->code) == 0) && (__tracker - __offset < get_event_size(event->nb_params))))
        to_be_loaded = 1;
    // raw event
    else if (((get_bit(event->code) == 1)
            && (__tracker - __offset
                    < get_event_size((evnt_size_t) ceil((double) event->nb_params / sizeof(evnt_param_t))))))
        to_be_loaded = 1;

    // fetch the next block of data from the trace
    if (to_be_loaded == 1) {
        *buffer = __next_trace();
        event = (evnt_t *) *buffer;
        __tracker = __offset + __buffer_size;
        to_be_loaded = 0;
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        *buffer = NULL;
        return NULL ;
    }

    // move pointer to the next event and update __offset
    if (get_bit(event->code) == 1)
        // raw event
        size = ceil((double) event->nb_params / sizeof(evnt_param_t));
    else
        // regular event
        size = event->nb_params;

    *buffer += get_event_components(size);
    __offset += get_event_size(size);

    return event;
}

/*
 * This function reads the next event from the buffer
 */
evnt_t* next_event(evnt_trace_t* buffer) {
    return read_event(buffer);
}

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(evnt_trace_t* buffer) {
    fclose(__ftrace);
    free(__buffer_ptr);

    // set pointers to NULL
    __buffer_ptr = NULL;
    *buffer = NULL;
}

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_t* event;
    evnt_trace_t buffer;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        perror("Specify the name of the trace file using '-f'\n");
        exit(EXIT_FAILURE);
    }

    buffer = open_trace(filename);
    printf("=============================================================\n");
    printf("Printing events from the %s file\n\n", filename);
    printf("Event Code \t Thread ID \t Time[ns] \t NB args \t Arguments[0-9]\n");

    while (buffer != NULL ) {
        event = read_event(&buffer);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            // regular event
            printf("%lx \t %lu \t %lu \t %lu", event->code, event->tid, event->time, event->nb_params);

            for (i = 0; i < event->nb_params; i++)
                printf("\t %lx", event->param[i]);
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%lx \t %lu \t %lu \t %lu", event->code, event->tid, event->time, event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    printf("=============================================================\n");
    close_trace(&buffer);

    return EXIT_SUCCESS;
}
