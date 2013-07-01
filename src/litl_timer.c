/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "litl_timer.h"

#define ERROR_TIMER_NOT_AVAILABLE() do {					\
    fprintf(stderr, "Trying to use timer function %s, but it is not available on this platform\n",__FUNCTION__); \
    abort();								\
  } while(0)

// Choose the default timing method
#ifdef CLOCK_GETTIME_AVAIL
#ifdef CLOCK_MONOTONIC_RAW
#define TIMER_DEFAULT litl_get_time_monotonic_raw
#else
#define TIMER_DEFAULT litl_get_time_monotonic
#endif	// CLOCK_MONOTONIC_RAW
#else  // CLOCK_GETTIME_AVAIL
#define TIMER_DEFAULT litl_get_time_ticks
#endif // CLOCK_GETTIME_AVAIL

/*
 * Selected timing method
 */
litl_timing_method_t litl_get_time = TIMER_DEFAULT;

/*
 * This function initializes the timing mechanism
 */
void litl_time_initialize() {
    char* time_str = getenv("LITL_TIMING_METHOD");
    if (time_str) {
        if (strcmp(time_str, "monotonic_raw") == 0)
            litl_set_timing_method(litl_get_time_monotonic_raw);
        else if (strcmp(time_str, "monotonic") == 0)
            litl_set_timing_method(litl_get_time_monotonic);
        else if (strcmp(time_str, "ticks") == 0)
            litl_set_timing_method(litl_get_time_ticks);
        else {
            fprintf(stderr, "Unknown timining method: '%s'\n", time_str);
            abort();
        }
    }
}

/*
 * This function returns -1 if none of timings is available. Otherwise, it returns 0
 */
int litl_set_timing_method(litl_timing_method_t callback) {
    if (!callback)
        return -1;

    litl_get_time = callback;
    return 0;
}

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
litl_time_t litl_get_time_monotonic_raw() {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC_RAW))
    litl_time_t time;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    time = 1000000000 * tp.tv_sec + tp.tv_nsec;
    return time;
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC)
 */
litl_time_t litl_get_time_monotonic() {
#ifdef CLOCK_GETTIME_AVAIL
    litl_time_t time;
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    time = 1000000000 * tp.tv_sec + tp.tv_nsec;
    return time;
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses CPU specific register (for instance, rdtsc for X86* processors)
 */
litl_time_t litl_get_time_ticks() {
#if HAVE_RDTSC
     // This is a copy of rdtscll function from asm/msr.h
#define ticks(val) do {					\
    uint32_t __a,__d;						\
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d));		\
    (val) = ((litl_time_t)__a) | (((litl_time_t)__d)<<32);	\
  } while(0)

#else
    ERROR_TIMER_NOT_AVAILABLE();
#define ticks(val) (val) = -1
#endif

    static int ticks_initialized = 0;
    static litl_time_t __ticks_per_sec = -1;
    if (!ticks_initialized) {
        litl_time_t init_start, init_end;
        ticks(init_start);
        usleep(1000000);
        ticks(init_end);

        __ticks_per_sec = init_end - init_start;
        ticks_initialized = 1;
    }

    litl_time_t time;
    ticks(time);

    return time * 1e9 / __ticks_per_sec;
}
