/*
 * timer.h
 *
 *  Created on: Mar 24, 2010
 *      Author: Roman Iakymchuk
 */

#ifndef TIMER_H_
#define TIMER_H_

#include "evnt_types.h"

/*
 * This function measures time in ns or clock ticks depending on the timer used
 */
evnt_time_t get_time();

/*
 * This function measures time in clock ticks
 */
evnt_time_t get_ticks();
evnt_time_t get_ticks_per_sec();

#endif /* TIMER_H_ */
