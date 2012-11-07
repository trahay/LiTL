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
 * This function opens a trace
 */
trace open_trace(const char*);

/*
 * This function closes the trace
 */
void close_trace(trace);

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

#endif /* EVNT_READ_H_ */
