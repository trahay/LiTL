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
static trace buffer_cur;
static uint64_t buffer_size = 32 * 1024; // 32 KB
// offset from the beginning of the trace file
static uint64_t offset = 0;
static uint64_t tracker = 32 * 1024; // offset + buffer_size

// TODO: implement an _init function where tracker would be set.

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
    buffer_cur = malloc(buffer_size);

    int res = fread(buffer_cur, buffer_size, 1, ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the top of the trace file to a buffer!");

    buffer_ptr = buffer_cur;
    return buffer_cur;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static trace _next_trace() {
    fseek(ftrace, offset, SEEK_SET);

    int res = fread(buffer_cur, buffer_size, 1, ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1))
        perror("Could not copy the next part of the trace file to a buffer!");

    buffer_ptr = buffer_cur;
    return buffer_cur;
}

/*
 * This function resets the trace
 */
void reset_trace(trace* buffer_cur) {
    *buffer_cur = buffer_ptr;
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

    /*// skip the EVNT_BUFFER_FLUSHED event
     if (event->code == EVNT_BUFFER_FLUSHED) {
     // move pointer to the next event and update offset
     *buffer += get_event_size(event->nb_args);
     offset += get_event_size(event->nb_args) * sizeof(uint64_t);
     read_event(buffer);
     }*/

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
 * This function searches for the next event
 */
evnt* next_event(trace* buffer_cur) {
    evnt* event;
    event = NULL;

    return event;
}

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(trace* buffer_cur) {
    fclose(ftrace);
    free(buffer_ptr);

    // set pointers to NULL
    buffer_ptr = NULL;
    *buffer_cur = NULL;
}
