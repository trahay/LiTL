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
      if (strcmp(time_str, "monotonic_raw") == 0) {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC_RAW))
            litl_set_timing_method(litl_get_time_monotonic_raw);
#else
	    goto not_available;
#endif
      } else if (strcmp(time_str, "monotonic") == 0)
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC))
            litl_set_timing_method(litl_get_time_monotonic);
#else
	    goto not_available;
#endif
        else if (strcmp(time_str, "realtime") == 0)
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_REALTIME))
            litl_set_timing_method(litl_get_time_realtime);
#else
	    goto not_available;
#endif
        else if (strcmp(time_str, "process_cputime") == 0)
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_PROCESS_CPUTIME_ID))
            litl_set_timing_method(litl_get_time_process_cputime);
#else
	    goto not_available;
#endif
        else if (strcmp(time_str, "thread_cputime") == 0)
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_THREAD_CPUTIME_ID))
            litl_set_timing_method(litl_get_time_thread_cputime);
#else
	    goto not_available;
#endif
        else if (strcmp(time_str, "ticks") == 0)
#if defined(__x86_64__) || defined(__i386)
            litl_set_timing_method(litl_get_time_ticks);
#else
	    goto not_available;
#endif
        else {
            fprintf(stderr, "Unknown timining method: '%s'\n", time_str);
            abort();
        }
    }
    return ;
 not_available:
    fprintf(stderr, "Timing function '%s' not available on this system\n", time_str);
    abort();
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

static inline litl_time_t __litl_get_time_generic(clockid_t clk_id) {
    litl_time_t time;
    struct timespec tp;
    clock_gettime(clk_id, &tp);
    time = 1000000000 * tp.tv_sec + tp.tv_nsec;
    return time;
}

/*
 * This function uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
litl_time_t litl_get_time_monotonic_raw() {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC_RAW))
    return __litl_get_time_generic(CLOCK_MONOTONIC_RAW);
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
    return __litl_get_time_generic(CLOCK_MONOTONIC);
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses clock_gettime(CLOCK_REALTIME)
 */
litl_time_t litl_get_time_realtime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined (CLOCK_REALTIME))
    return __litl_get_time_generic(CLOCK_REALTIME);
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses clock_gettime(CLOCK_PROCESS_CPUTIME_ID)
 */
litl_time_t litl_get_time_process_cputime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined (CLOCK_PROCESS_CPUTIME_ID))
    return __litl_get_time_generic(CLOCK_PROCESS_CPUTIME_ID);
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses clock_gettime(CLOCK_THREAD_CPUTIME_ID)
 */
litl_time_t litl_get_time_thread_cputime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined(CLOCK_THREAD_CPUTIME_ID))
    return __litl_get_time_generic(CLOCK_THREAD_CPUTIME_ID);
#else
    ERROR_TIMER_NOT_AVAILABLE();
    return -1;
#endif
}

/*
 * This function uses CPU specific register (for instance, rdtsc for X86* processors)
 */
litl_time_t litl_get_time_ticks() {

#ifdef __x86_64__
     // This is a copy of rdtscll function from asm/msr.h
#define ticks(val) do {					\
    uint32_t __a,__d;						\
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d));		\
    (val) = ((litl_time_t)__a) | (((litl_time_t)__d)<<32);	\
  } while(0)

#elif defined(__i386)

#define ticks(val) \
  __asm__ volatile("rdtsc" : "=A" (val))

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
