/*
 * evnt_read.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_READ_H_
#define EVNT_READ_H_

/*
 * This functions opens a trace
 */
Trace* open_trace(char* filename);

/*
 * This functions closes the trace
 */
void close_trace(Trace* trace);

/*
 * This function searches for the next event
 */
evnt* next_event(Trace* trace);

#endif /* EVNT_READ_H_ */
