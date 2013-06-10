/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_TIMER_H_
#define LITL_TIMER_H_

#include "litl_types.h"

/*
 * Callback function that return the current time in ns.
 * It can be either a pointer to one of the timing functions
 * provided by LiTL, or a user-defined function.
 */
typedef litl_time_t (*litl_timing_method_t)();

/*
 * This function calls the selected timing method and get the current time in ns
 */
extern litl_timing_method_t litl_get_time;

/*
 * This function initializes the timing mechanism
 */
void litl_time_initialize();

/*
 * Select the timing function to use.
 * Return -1 if an error occurs
 */
int litl_set_timing_method(litl_timing_method_t callback);

// pre-defined timing methods:

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
litl_time_t litl_get_time_monotonic_raw();

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC)
 */
litl_time_t litl_get_time_monotonic();

/*
 * This function uses CPU specific register (for instance, rdtsc for X86* processors)
 */
litl_time_t litl_get_time_ticks();

#endif /* LITL_TIMER_H_ */
