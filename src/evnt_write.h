/*
 * evnt_write.h
 *
 *  Created on: Oct 29, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_WRITE_H_
#define EVNT_WRITE_H_

#include "evnt_types.h"

/*
 * This function initializes the trace
 */
void init_trace(char* filename, buffer_flags buffer_flush, thread_flags thread_safe, uint32_t buffer_size);

/*
 * This function finalizes the trace
 */
void finalize_trace();

/*
 * This function records an event without any arguments
 */
void evnt_probe0(uint64_t);

/*
 * This function records an event with one argument
 */
void evnt_probe1(uint64_t, uint64_t);

/*
 * This function records an event with two arguments
 */
void evnt_probe2(uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with three arguments
 */
void evnt_probe3(uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with four arguments
 */
void evnt_probe4(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with five arguments
 */
void evnt_probe5(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with six arguments
 */
void evnt_probe6(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

/*
 * This function records an event in a raw state.
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t, uint64_t, void*); // there is a possibility to remove size

/*
 * This function writes the recorded events from the buffer to the file
 */
void _buffer_flush();

#endif /* EVNT_WRITE_H_ */
