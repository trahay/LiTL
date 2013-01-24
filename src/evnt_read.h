/*
 * evnt_read.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

#include "evnt_types.h"

/*
 * This function sets the buffer size for reading the trace file
 */
void set_read_buffer_size(const uint32_t);

/*
 * This function returns the current trace, FILE pointer, and the current position in the file
 */
evnt_block_t get_evnt_block(evnt_trace_t);

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
evnt_trace_t open_trace(const char*);

/*
 * This function resets the trace
 */
//void reset_trace(evnt_trace_t*);

/*
 * This function reads an event
 */
evnt_t* read_event(evnt_block_t*);

/*
 * This function searches for the next event
 */
evnt_t* next_event(evnt_block_t*);

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(evnt_block_t*);

#endif /* EVNT_READ_H_ */
