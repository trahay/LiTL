/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "evnt_types.h"

/* time in nanosecond */
typedef uint64_t evnt_time_t;


/* Callback function that return the current time in ns.
 * It can be either a pointer to one of the timing functions
 * provided by libevnt, or a user-defined function.
 */
typedef evnt_time_t (*evnt_timing_method_t)() ;


/* Calls the selected timing method and get the current time in ns */
extern evnt_timing_method_t evnt_get_time;


/* Function to initialize the timing mechanism */
void evnt_time_initialize();

/* Select the timing function to use.
 * Return -1 if an error occurs
 */
int evnt_set_timing_method(evnt_timing_method_t callback);




/*** pre-defined timing methods: ***/

/* uses clock_gettime(CLOCK_MONOTONIC_RAW) */
evnt_time_t evnt_get_time_monotonic_raw();

/* uses clock_gettime(CLOCK_MONOTONIC) */
evnt_time_t evnt_get_time_monotonic();

/* uses CPU specific register (for instance, rdtsc for X86* processors) */
evnt_time_t evnt_get_time_ticks();

#endif /* TIMER_H_ */
