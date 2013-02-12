/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include "evnt_macro.h"
#include "evnt_read.h"

static FILE* __ftrace;
static uint32_t __buffer_size = 16 * 1024 * 1024; // 16MB is the optimal size on Intel Core 2

/*
 * This function sets the buffer size
 */
void set_read_buffer_size(const uint32_t buf_size) {
    __buffer_size = buf_size;
}

/*
 * This function returns the current trace, FILE pointer, and the current position in the file
 */
evnt_block_t get_evnt_block(evnt_trace_t trace) {
    evnt_block_t block;

    block.fp = __ftrace;
    block.trace_head = trace;
    block.trace = trace;
    block.offset = 0;
    block.tracker = __buffer_size;

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

    evnt_trace_t buffer = (evnt_trace_t) malloc(__buffer_size);

    int res = fread(buffer, __buffer_size, 1, __ftrace);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not copy the top of the trace file to a buffer!");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static __next_trace(evnt_block_t* block) {
    fseek(block->fp, block->offset, SEEK_SET);

    int res = fread(block->trace_head, __buffer_size, 1, block->fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not copy the next part of the trace file to a buffer!");
        exit(EXIT_FAILURE);
    }

    block->trace = block->trace_head;
    block->tracker = block->offset + __buffer_size;
}

/*
 * This function resets the trace
 */
void reset_trace(evnt_block_t* block) {
    block->trace = block->trace_head;
}

/*
 * This function reads an event
 */
evnt_t* read_event(evnt_block_t* block) {
    uint8_t to_be_loaded;
    evnt_size_t size;
    evnt_t* event;
    evnt_trace_t* buffer;
    buffer = &block->trace;
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
    // regular event
    if ((block->tracker - block->offset < get_event_size(0))
            || ((get_bit(event->code) == 0) && (block->tracker - block->offset < get_event_size(event->nb_params))))
        to_be_loaded = 1;
    // raw event
    else if ((get_bit(event->code) == 1)
            && (block->tracker - block->offset
                    < get_event_size((evnt_size_t) ceil((double) event->nb_params / sizeof(evnt_param_t)))))
        to_be_loaded = 1;

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __next_trace(block);
        buffer = &block->trace;
        event = (evnt_t *) *buffer;
        to_be_loaded = 0;
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        block->trace = NULL;
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
    block->offset += get_event_size(size);

    return event;
}

/*
 * This function reads the next event from the buffer
 */
evnt_t* next_event(evnt_block_t* block) {
    return read_event(block);
}

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(evnt_block_t* block) {
    fclose(block->fp);
    free(block->trace_head);

    // set pointers to NULL
    block->fp = NULL;
    block->trace = NULL;
    block->trace_head = NULL;
}

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_t* event;
    evnt_trace_t buffer;
    evnt_block_t block;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        perror("Specify the name of the trace file using '-f'\n");
        exit(EXIT_FAILURE);
    }

    buffer = open_trace(filename);
    block = get_evnt_block(buffer);

    while (block.trace != NULL ) {
        event = read_event(&block);

        if (event == NULL )
            break;

        if (get_bit(event->code) == 0) {
            // regular event
            printf("%"PRIx32" \t %"PRIu64" \t %"PRIu64" \t %"PRIu32, event->code, event->tid, event->time, event->nb_params);

            for (i = 0; i < event->nb_params; i++)
                printf("\t %"PRIx64, event->param[i]);
        } else {
            // raw event
            event->code = clear_bit(event->code);

            printf("%"PRIx32" \t %"PRIu64" \t %"PRIu64" \t %"PRIu32, event->code, event->tid, event->time, event->nb_params);
            printf("\t %s", (evnt_data_t *) event->param);
        }

        printf("\n");
    }

    close_trace(&block);

    return EXIT_SUCCESS;
}
