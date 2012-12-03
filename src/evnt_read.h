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
 * This function opens a trace and reads the first portion of data to the buffer
 */
trace_t open_trace(const char*);

/*
 * This function resets the trace
 */
void reset_trace(trace_t*);

/*
 * This function reads an event
 */
evnt_t* read_event(trace_t*);

/*
 * This function searches for the next event
 */
evnt_t* next_event(trace_t*);

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(trace_t*);

#endif /* EVNT_READ_H_ */
