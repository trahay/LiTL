/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "evnt_types.h"

/*
 * Callback function that return the current time in ns.
 * It can be either a pointer to one of the timing functions
 * provided by libevnt, or a user-defined function.
 */
typedef evnt_time_t (*evnt_timing_method_t)();

/*
 * This function calls the selected timing method and get the current time in ns
 */
extern evnt_timing_method_t evnt_get_time;

/*
 * This function initializes the timing mechanism
 */
void evnt_time_initialize();

/*
 * Select the timing function to use.
 * Return -1 if an error occurs
 */
int evnt_set_timing_method(evnt_timing_method_t callback);

// pre-defined timing methods:

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
evnt_time_t evnt_get_time_monotonic_raw();

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC)
 */
evnt_time_t evnt_get_time_monotonic();

/*
 * This function uses CPU specific register (for instance, rdtsc for X86* processors)
 */
evnt_time_t evnt_get_time_ticks();

#endif /* TIMER_H_ */
