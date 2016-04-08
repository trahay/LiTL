/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_TIMER_H_
#define LITL_TIMER_H_

/**
 *  \file litl_timer.h
 *  \brief litl_timer Provides a set of functions for measuring time
 *
 *  \authors
 *    Developers are : \n
 *        Roman Iakymchuk   -- roman.iakymchuk@telecom-sudparis.eu \n
 *        Francois Trahay   -- francois.trahay@telecom-sudparis.eu \n
 */

#include "litl_types.h"

/**
 * \defgroup litl_timer LiTL Timing Methods
 */

/**
 * \defgroup litl_timer_init Initialization Functions
 * \ingroup litl_timer
 */

/**
 * \defgroup litl_timer_measure Pre-Defined Timing Methods
 * \ingroup litl_timer
 */

/**
 * \ingroup litl_timer_init
 * \brief A callback function that returns the current time in ns. It can be
 * either a pointer to one of the timing functions provided by LiTL or a
 * user-defined function
 */
typedef litl_time_t (*litl_timing_method_t)();

/**
 * \ingroup litl_timer_measure
 * \brief Calls the selected timing method and get the current time in ns
 */
extern litl_timing_method_t litl_get_time;

/**
 * \ingroup litl_timer_init
 * \brief Initializes the timing mechanism
 */
void litl_time_initialize();

/**
 * \ingroup litl_timer_init
 * \brief Selects the timing function to use
 * \param callback A name of timing function
 * \return Returns -1 if an error occurs. Otherwise, returns 0
 */
int litl_set_timing_method(litl_timing_method_t callback);

// Pre-defined timing methods:

/**
 * \ingroup litl_timer_measure
 * \brief Uses clock_gettime(CLOCK_MONOTONIC_RAW)
 * \return Returns time that is similar to CLOCK_MONOTONIC, but provides access
 *  to a raw hardware-based time
 */
litl_time_t litl_get_time_monotonic_raw();

/**
 * \ingroup litl_timer_measure
 * \brief Uses clock_gettime(CLOCK_MONOTONIC)
 * \return Returns the monotonic time since some unspecified starting point
 */
litl_time_t litl_get_time_monotonic();

/**
 * \ingroup litl_timer_measure
 * \brief Uses clock_gettime(CLOCK_REALTIME)
 * \return Returns the real (wall-clock) time
 */
litl_time_t litl_get_time_realtime();

/**
 * \ingroup litl_timer_measure
 * \brief Uses clock_gettime(CLOCK_PROCESS_CPUTIME)
 * \return Returns the high-resolution per-process time from the CPU
 */
litl_time_t litl_get_time_process_cputime();

/**
 * \ingroup litl_timer_measure
 * \brief Uses clock_gettime(CLOCK_THREAD_CPUTIME)
 * \return Returns the thread-specific CPU-time
 */
litl_time_t litl_get_time_thread_cputime();

/**
 * \ingroup litl_timer_measure
 * \brief Uses CPU-specific register (for instance, rdtsc for X86* processors)
 * \return Returns the measured time in clock cycles
 */
litl_time_t litl_get_time_ticks();

/**
 * \ingroup litl_timer_measure
 * \brief Ultra-fast measurement function
 * \return Always returns 0
 */
litl_time_t litl_get_time_none();

#endif /* LITL_TIMER_H_ */
