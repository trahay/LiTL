/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_WRITE_H_
#define EVNT_WRITE_H_

#include "evnt_types.h"

void evnt_buffer_flush_on();
void evnt_buffer_flush_off();

void evnt_thread_safety_on();
void evnt_thread_safety_off();

void evnt_tid_recording_on();
void evnt_tid_recording_off();

void evnt_set_filename(char*);

/*
 * This function initializes the trace
 */
void evnt_init_trace(const uint32_t);

/*
 * This function finalizes the trace
 */
void evnt_fin_trace();

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void evnt_flush_buffer();

/*
 * This function records an event without any arguments
 */
void evnt_probe0(evnt_code_t);

/*
 * This function records an event with one argument
 */
void evnt_probe1(evnt_code_t, evnt_param_t);

/*
 * This function records an event with two arguments
 */
void evnt_probe2(evnt_code_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with three arguments
 */
void evnt_probe3(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with four arguments
 */
void evnt_probe4(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with five arguments
 */
void evnt_probe5(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with six arguments
 */
void evnt_probe6(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t);

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t);

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event with ten arguments
 */
void evnt_probe10(evnt_code_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t,
        evnt_param_t, evnt_param_t, evnt_param_t, evnt_param_t);

/*
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(evnt_code_t, evnt_size_t, evnt_data_t[]);

#endif /* EVNT_WRITE_H_ */
