/*
 * evnt_read.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

/*
 * This function opens a trace
 */
Trace* open_trace(char* filename);

/*
 * This function closes the trace
 */
void close_trace(Trace* trace);

/*
 * This function resets the trace
 */
void reset_trace(Trace* trace);

/*
 * This function reads an event
 */
evnt* read_event(Trace* trace);

/*
 * This function searches for the next event
 */
evnt* next_event(Trace* trace);

#endif /* EVNT_READ_H_ */
