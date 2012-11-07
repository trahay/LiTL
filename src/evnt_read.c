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

static trace buffer_ptr;

/*
 * This function opens a binary trace file
 */
trace open_trace(const char* file_path) {
    FILE *ftrace;
    trace buffer_cur;
    struct stat st;

    if (!(ftrace = fopen(file_path, "r")))
        perror("Could not open the trace file for reading!");

    if (stat(file_path, &st) != 0)
        perror("Could not get the attributes of the trace file!");

    buffer_cur = malloc(st.st_size);
    // TODO: there is a potential to being not able to read the whole trace at once. Alternative solution is needed
    if (fread(buffer_cur, st.st_size, 1, ftrace) != 1)
        perror("Could not copy the content of the trace file to a buffer!");

    buffer_ptr = buffer_cur;
    fclose(ftrace);

    return buffer_cur;
}

/*
 * This function closes the trace
 */
void close_trace(trace buffer_cur) {
//    free(buffer_cur);
    free(buffer_ptr);
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
evnt* read_event(trace* buffer_cur) {
    evnt* event;

    if (buffer_cur == NULL )
        return NULL ;

    event = malloc(sizeof(evnt));
    event = (evnt *) *buffer_cur;

    // skip events like EVNT_BUFFER_FLUSHED and EVNT_TRACE_END
    if (event->code == EVNT_BUFFER_FLUSHED) {
        *buffer_cur += get_event_size(event->nb_args);
        read_event(buffer_cur);
    }

    if (event->code == EVNT_TRACE_END)
        return NULL ;

    // move pointer to the other event
    *buffer_cur += get_event_size(event->nb_args);

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
