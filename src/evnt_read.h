/*
 * evnt_read.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

#include "evnt_types.h"

void set_buffer_size(const uint64_t);

/*
 * This function opens a trace and reads the first portion of data to the buffer
 */
trace open_trace(const char*);

/*
 * This function resets the trace
 */
void reset_trace(trace*);

/*
 * This function reads an event
 */
evnt* read_event(trace*);

/*
 * This function searches for the next event
 */
evnt* next_event(trace*);

/*
 * This function closes the trace and frees the buffer
 */
void close_trace(trace*);

#endif /* EVNT_READ_H_ */
