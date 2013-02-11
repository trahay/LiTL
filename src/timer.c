/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <time.h>

#include "timer.h"

static evnt_time_t __ticks_per_sec;

/*
 * This is a copy of rdtscll function from asm/msr.h
 */
#define rdtscll(val) do { \
      uint32_t __a,__d; \
      asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
      (val) = ((evnt_time_t)__a) | (((evnt_time_t)__d)<<32); \
} while(0)

/*
 * This function initializes __ticks_per_sec
 */
void __init_time() __attribute__((constructor));
void __init_time() {
    evnt_time_t init_start, init_end;
    rdtscll(init_start);
    usleep(1000000);
    rdtscll(init_end);

    __ticks_per_sec = init_end - init_start;
//    printf("1 second is %lu clock ticks\n", __ticks_per_sec);
//    printf("machine seems to run with %.2lf GHz\n", __ticks_per_sec / 1000000000.0);
}

/*
 * This function measures time in ns or clock ticks depending on the timer used
 */
evnt_time_t get_time() {
    evnt_time_t time;

#ifdef CLOCK_GETTIME_AVAIL
    struct timespec tp;
#ifdef CLOCK_MONOTONIC_RAW
    // avoid NTP
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
#else
    clock_gettime(CLOCK_MONOTONIC, &tp);
#endif
    time = 1000000000 * tp.tv_sec + tp.tv_nsec;
#else
    time = get_ticks();
#endif

    return time;
}

/*
 * This function measures time in clock ticks
 */
evnt_time_t get_ticks() {
    evnt_time_t time;
    rdtscll(time);
    return time;
}

evnt_time_t get_ticks_per_sec() {
    return __ticks_per_sec;
}
