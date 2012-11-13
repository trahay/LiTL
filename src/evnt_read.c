/*
 * evnt_read.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdlib.h>
#include <sys/stat.h>

#include "evnt_macro.h"
#include "evnt_read.h"

static FILE *ftrace;
static trace buffer_ptr;
static uint32_t buffer_size;
// offset from the beginning of the trace file
static uint32_t offset;
static uint32_t tracker;

/*
 * This function initialize tracker using offset and buffer_size
 */
void _init_tracker() __attribute__((constructor));

void _init_tracker() {
    offset = 0;
    buffer_size = 256 * 1024; // 256KB is the optimal size on Intel Core 2
    tracker = offset + buffer_size;
}

void set_read_buffer_size(const uint32_t buf_size) {
    offset = 0;
    buffer_size = buf_size;
    tracker = offset + buffer_size;
}

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
trace open_trace(const char* file_path) {
    struct stat st;

    if (!(ftrace = fopen(file_path, "r")))
        perror("Could not open the trace file for reading!");

    if (stat(file_path, &st) != 0)
        perror("Could not get the attributes of the trace file!");

    if (buffer_size > st.st_size)
        buffer_size = st.st_size;
    buffer_ptr = malloc(buffer_size);

    int res = fread(buffer_ptr, buffer_size, 1, ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the top of the trace file to a buffer!");

    return buffer_ptr;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static trace _next_trace() {
    fseek(ftrace, offset, SEEK_SET);

    int res = fread(buffer_ptr, buffer_size, 1, ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the next part of the trace file to a buffer!");

    return buffer_ptr;
}

/*
 * This function resets the trace
 */
void reset_trace(trace* buffer) {
    *buffer = buffer_ptr;
}

/*
 * This function reads an event
 */
evnt* read_event(trace* buffer) {
    evnt* event;

    if (*buffer == NULL )
        return NULL ;

    event = (evnt *) *buffer;

    // While reading events from the buffer, there can be two situations:
    // 1. The situation when the buffer contains exact number of events;
    // 2. The situation when only part of the last event is loaded to the buffer.
    //    Then, nb_args is checked whether it is defined or not.
    //    If nb_args is OK, then the event is studied on whether it is loaded completely or not.
    //    If any of these cases is not true, the next part of the trace plus the current event is loaded to the buffer.
    if (tracker - offset <= sizeof(evnt)) {
        if ((event->nb_args > 9) || (tracker - offset < get_event_size(event->nb_args) * sizeof(uint64_t))) {
            *buffer = _next_trace();
            event = (evnt *) *buffer;
            tracker = offset + buffer_size;
        }
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        *buffer = NULL;
        return NULL ;
    }

    // move pointer to the next event and update offset
    *buffer += get_event_size(event->nb_args);
    offset += get_event_size(event->nb_args) * sizeof(uint64_t);

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
    fclose(ftrace);
    free(buffer_ptr);

    // set pointers to NULL
    buffer_ptr = NULL;
    *buffer = NULL;
}
