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
 * This function sets the buffer size
 */
void set_write_buffer_size(const uint32_t);

/*
 * This function sets the buffer flush flag
 */
void set_buffer_flag(const buffer_flags);

/*
 * This function sets the thread safe flag
 */
void set_thread_flag(const thread_flags);

/*
 * This function initializes the trace
 */
void init_trace(const char*, const buffer_flags, const thread_flags, const uint32_t);

/*
 * This function finalizes the trace
 */
void fin_trace();

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void flush_buffer();

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
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t, uint64_t, void*);

#endif /* EVNT_WRITE_H_ */
