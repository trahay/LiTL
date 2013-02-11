/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "evnt_types.h"

#ifndef CLOCK_GETTIME_AVAIL
/*
 * This function measures time in clock ticks
 */
evnt_time_t get_ticks();
evnt_time_t get_ticks_per_sec();
#endif

/*
 * This function measures time in ns or clock ticks depending on the timer used
 */
evnt_time_t get_time();

#endif /* TIMER_H_ */
