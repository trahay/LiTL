/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

#include "evnt_types.h"

/*
 * This function sets the buffer size for reading the trace file
 */
void set_read_buffer_size(const uint32_t);

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_buffer_t evnt_open_trace(const char*);

/*
 * This function reads information contained in the trace header
 */
evnt_info_t* evnt_get_trace_header(evnt_block_t*);

/*
 * This function returns the current trace, FILE pointer, and the current position in the file
 */
evnt_block_t evnt_get_block(evnt_buffer_t);

/*
 * This function resets the trace
 */
void evnt_reset_trace(evnt_block_t*);

/*
 * This function reads an event
 */
evnt_t* evnt_read_event(evnt_block_t*);

/*
 * This function searches for the next event
 */
evnt_t* evnt_next_event(evnt_block_t*);

/*
 * This function closes the trace and frees the buffer
 */
void evnt_close_trace(evnt_block_t*);

#endif /* EVNT_READ_H_ */
