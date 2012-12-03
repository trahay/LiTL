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
static trace __buffer_ptr;
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

void set_read_buffer_size(const uint32_t buf_size) {
    __offset = 0;
    __buffer_size = buf_size;
    __tracker = __offset + __buffer_size;
}

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
trace open_trace(const char* file_path) {
    struct stat st;

    if (!(__ftrace = fopen(file_path, "r")))
        perror("Could not open the trace file for reading!");

    if (stat(file_path, &st) != 0)
        perror("Could not get the attributes of the trace file!");

    if (__buffer_size > st.st_size)
        __buffer_size = st.st_size;
    __buffer_ptr = malloc(__buffer_size);

    int res = fread(__buffer_ptr, __buffer_size, 1, __ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the top of the trace file to a buffer!");

    return __buffer_ptr;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static trace __next_trace() {
    fseek(__ftrace, __offset, SEEK_SET);

    int res = fread(__buffer_ptr, __buffer_size, 1, __ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the next part of the trace file to a buffer!");

    return __buffer_ptr;
}

/*
 * This function resets the trace
 */
void reset_trace(trace* buffer) {
    *buffer = __buffer_ptr;
}

/*
 * This function reads an event
 */
evnt* read_event(trace* buffer) {
    uint8_t to_be_loaded = 0;
    evnt_size_t size;
    evnt* event;

    if (*buffer == NULL )
        return NULL;

    event = (evnt *) *buffer;

    /* While reading events from the buffer, there can be two situations:
     1. The situation when the buffer contains exact number of events;
     2. The situation when only a part of the last event is loaded to the buffer.
     Check whether the main four components (tid, time, code, nb_args) are loaded.
     Check whether all arguments are loaded.
     If any of these cases is not true, the next part of the trace plus the current event is loaded to the buffer.*/
    // regular event
    if ((__tracker - __offset <= sizeof(evnt))
            || ((get_bit(event->code) == 0) && (__tracker - __offset < get_event_size(event->nb_args))))
        to_be_loaded = 1;
    // raw event
    else if (((get_bit(event->code) == 1)
            && (__tracker - __offset < get_event_size((evnt_size_t) ceil((double) event->nb_args / sizeof(evnt_args_t))))))
        to_be_loaded = 1;

    // fetch the next block of data from the trace
    if (to_be_loaded == 1) {
        *buffer = __next_trace();
        event = (evnt *) *buffer;
        __tracker = __offset + __buffer_size;
        to_be_loaded = 0;
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        *buffer = NULL;
        return NULL;
    }

    // move pointer to the next event and update __offset
    if (get_bit(event->code) == 1)
        // raw event
        size = ceil((double) event->nb_args / sizeof(evnt_args_t));
    else
        // regular event
        size = event->nb_args;

    *buffer += get_event_components(size);
    __offset += get_event_size(size);

    return event;
}

/*
 * This function reads the next event from the buffer
 */
evnt* next_event(trace* buffer) {
    return read_event(buffer);
}

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(trace* buffer) {
    fclose(__ftrace);
    free(__buffer_ptr);

    // set pointers to NULL
    __buffer_ptr = NULL;
    *buffer = NULL;
}
