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

static FILE* __ftrace;
static uint32_t __buffer_size = 16 * 1024 * 1024; // 16MB is the optimal size on Intel Core 2

/*
 * This function sets the buffer size
 */
void set_read_buffer_size(const uint32_t buf_size) {
    __buffer_size = buf_size;
}

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_buffer_t evnt_open_trace(const char* filename) {
    struct stat st;

    if (!(__ftrace = fopen(filename, "r"))) {
        perror("Could not open the trace file for reading!");
        exit(EXIT_FAILURE);
    }

    if (stat(filename, &st) != 0)
        perror("Could not get the attributes of the trace file!");

    if (__buffer_size > st.st_size)
        __buffer_size = st.st_size;

    evnt_buffer_t buffer = (evnt_buffer_t) malloc(__buffer_size);

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
 * This function reads information contained in the trace header
 */
evnt_info_t* evnt_get_trace_header(evnt_block_t* block) {
    evnt_size_t size;
    evnt_info_t* header;
    evnt_buffer_t* buffer;
    buffer = &block->buffer;

    header = (evnt_info_t *) *buffer;
    // +2 corresponds for the '\0' symbol after each string
    size = strlen(header->libevnt_ver) + strlen(header->sysinfo) + 2;

    size = (evnt_size_t) ceil((double) size / sizeof(evnt_param_t));
    *buffer += size;
    block->offset += size * sizeof(evnt_param_t);

    return header;
}

/*
 * This function returns the current buffer, FILE pointer, and the current position in the file
 */
evnt_block_t evnt_get_block(evnt_buffer_t buffer) {
    evnt_block_t block;

    block.fp = __ftrace;
    block.buffer_ptr = buffer;
    block.buffer = buffer;
    block.offset = 0;
    block.tracker = __buffer_size;

    return block;
}

/*
 * This function reads another portion of data from the trace file to the buffer
 */
static void __next_trace(evnt_block_t* block) {
    fseek(block->fp, block->offset, SEEK_SET);

    int res = fread(block->buffer_ptr, __buffer_size, 1, block->fp);
    // If the end of file is reached, then all data are read. So, res is 0.
    // Otherwise, res is either an error or the number of elements, which is 1.
    if ((res != 0) && (res != 1)) {
        perror("Could not copy the next part of the trace file to a buffer!");
        exit(EXIT_FAILURE);
    }

    block->buffer = block->buffer_ptr;
    block->tracker = block->offset + __buffer_size;
}

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_block_t* block) {
    block->buffer = block->buffer_ptr;
}

/*
 * This function reads an event
 */
evnt_t* evnt_read_event(evnt_block_t* block) {
    uint8_t to_be_loaded;
    evnt_size_t size;
    evnt_t* event;
    evnt_buffer_t* buffer;
    buffer = &block->buffer;
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

    unsigned remaining_size = block->tracker - block->offset;
    if(remaining_size < get_event_size(0)) {
      /* this event is truncated. We can't even read the nb_param field */
      to_be_loaded= 1;
    } else {
      /* The nb_param (or size) field is available. Let's see if the event is truncated */
      unsigned event_size = get_event_size_type(event);
      if(remaining_size < event_size)
	to_be_loaded = 1;
    }

    // fetch the next block of data from the trace
    if (to_be_loaded) {
        __next_trace(block);
        buffer = &block->buffer;
        event = (evnt_t *) *buffer;
        to_be_loaded = 0;
    }

    // skip the EVNT_TRACE_END event
    if (event->code == EVNT_TRACE_END) {
        block->buffer = NULL;
        *buffer = NULL;
        return NULL ;
    }

    // move pointer to the next event and update __offset
    unsigned evt_size = get_event_size_type(event);
    block->buffer = (evnt_buffer_t) (((uint8_t*)block->buffer) + evt_size);
    block->offset = (((uint8_t*)block->offset) + get_event_size_type(event));

    return event;
}

/*
 * This function reads the next event from the buffer
 */
evnt_t* evnt_next_event(evnt_block_t* block) {
    return evnt_read_event(block);
}

/*
 * This function closes the trace and frees the buffer
 */
void evnt_close_trace(evnt_block_t* block) {
    fclose(block->fp);
    free(block->buffer_ptr);

    // set pointers to NULL
    block->fp = NULL;
    block->buffer = NULL;
    block->buffer_ptr = NULL;
}

int main(int argc, const char **argv) {
    evnt_size_t i;
    const char* filename = "trace";
    evnt_t* event;
    evnt_buffer_t buffer;
    evnt_block_t block;
    evnt_info_t* header;

    if ((argc == 3) && (strcmp(argv[1], "-f") == 0))
        filename = argv[2];
    else {
        perror("Specify the name of the trace file using '-f'\n");
        exit(EXIT_FAILURE);
    }

    buffer = evnt_open_trace(filename);
    block = evnt_get_block(buffer);
    header = evnt_get_trace_header(&block);

    // print the header
    printf(" libevnt v.%s\n", header->libevnt_ver);
    printf(" %s\n", header->sysinfo);

    while (block.buffer != NULL ) {
        event = evnt_read_event(&block);

        if (event == NULL ) {
            break;
	}

	switch(event->type) {
	case EVENT_TYPE_REGULAR:
	  { // regular event
            printf("Reg\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, event->tid, event->time,
                    event->parameters.regular.nb_params);

            for (i = 0; i < event->parameters.regular.nb_params; i++)
                printf("\t %"PRTIx64, event->parameters.regular.param[i]);
	    break;
	  }
	case EVENT_TYPE_RAW:
	  { // raw event
            event->code = clear_bit(event->code);
            printf("Raw\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, event->tid, event->time,
                    event->parameters.raw.size);
            printf("\t %s", (evnt_data_t *) event->parameters.raw.data);
	    break;
	  }
	case EVENT_TYPE_PACKED:
	  { // packed event
            printf("Packed\t %"PRTIx32" \t %"PRTIu64" \t %"PRTIu64" \t %"PRTIu32, event->code, event->tid, event->time,
		   event->parameters.packed.size);
	    for(i=0; i<event->parameters.packed.size; i++) {
	      printf("\t%x",event->parameters.packed.param[i]);
	    }
	    break;
	  }
	default:
	  {
	    fprintf(stderr, "Unknown event type %d\n", event->type);
	    *(int*)0=0;
	  }
	}

        printf("\n");
    }

    evnt_close_trace(&block);

    return EXIT_SUCCESS;
}
