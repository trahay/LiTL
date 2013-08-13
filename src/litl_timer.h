/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_TIMER_H_
#define LITL_TIMER_H_

#include "litl_types.h"

/*
 * A callback function that returns the current time in ns.
 * It can be either a pointer to one of the timing functions
 * provided by LiTL or a user-defined function
 */
typedef litl_time_t (*litl_timing_method_t)();

/*
 * Calls the selected timing method and get the current time in ns
 */
extern litl_timing_method_t litl_get_time;

/*
 * Initializes the timing mechanism
 */
void litl_time_initialize();

/*
 * Selects the timing function to use.
 * Return -1 if an error occurs
 */
int litl_set_timing_method(litl_timing_method_t callback);

// Pre-defined timing methods:

/*
 * Uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
litl_time_t litl_get_time_monotonic_raw();

/*
 * Uses clock_gettime(CLOCK_MONOTONIC)
 */
litl_time_t litl_get_time_monotonic();

/*
 * Uses clock_gettime(CLOCK_REALTIME)
 */
litl_time_t litl_get_time_realtime();

/*
 * Uses clock_gettime(CLOCK_PROCESS_CPUTIME)
 */
litl_time_t litl_get_time_process_cputime();

/*
 * Uses clock_gettime(CLOCK_THREAD_CPUTIME)
 */
litl_time_t litl_get_time_thread_cputime();

/*
 * Uses CPU specific register (for instance, rdtsc for X86* processors)
 */
litl_time_t litl_get_time_ticks();

#endif /* LITL_TIMER_H_ */
