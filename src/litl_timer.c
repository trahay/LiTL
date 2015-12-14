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

litl_time_t litl_get_time_none();

#define ERROR_TIMER_NOT_AVAILABLE() do {				\
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
 * Benchmarks function f and returns the number of calls to f that can be done
 *   in 100 microseconds
 */
static unsigned __litl_time_benchmark_generic(litl_timing_method_t f) {
  unsigned i = 0;
  unsigned threshold = 100000; // how many calls to f() in 100 microseconds ?
  litl_time_t t1, t2;
  t1 = f();
  do {
    t2 = f();
    i++;
  } while (t2 - t1 < threshold);

  return i;
}

/*
 * Selects the most efficient timing method
 */
static void __litl_time_benchmark() {
  unsigned best_score = 0;
  unsigned cur_score;

#define RUN_BENCHMARK(_func_) do {			\
    cur_score = __litl_time_benchmark_generic(_func_);	\
    if(cur_score > best_score) {			\
      best_score = cur_score;				\
      litl_set_timing_method(_func_);			\
    }							\
  }while(0)

#ifdef CLOCK_GETTIME_AVAIL

#ifdef CLOCK_MONOTONIC_RAW
  RUN_BENCHMARK(litl_get_time_monotonic_raw);
#endif

#ifdef CLOCK_MONOTONIC
  RUN_BENCHMARK(litl_get_time_monotonic);
#endif

#ifdef CLOCK_REALTIME
  RUN_BENCHMARK(litl_get_time_realtime);
#endif

#ifdef CLOCK_PROCESS_CPUTIME_ID
  RUN_BENCHMARK(litl_get_time_process_cputime);
#endif

#ifdef CLOCK_THREAD_CPUTIME_ID
  RUN_BENCHMARK(litl_get_time_thread_cputime);
#endif

#endif	/* CLOCK_GETTIME_AVAIL */

#if defined(__x86_64__) || defined(__i386)
  RUN_BENCHMARK(litl_get_time_ticks);
#endif

  printf("[LiTL] selected timing method:");
#ifdef CLOCK_GETTIME_AVAIL
#ifdef CLOCK_MONOTONIC_RAW
  if(litl_get_time == litl_get_time_monotonic_raw)
    printf("monotonic_raw\n");
#endif

#ifdef CLOCK_MONOTONIC
  if(litl_get_time == litl_get_time_monotonic)
    printf("monotonic\n");
#endif

#ifdef CLOCK_REALTIME
  if(litl_get_time == litl_get_time_realtime)
    printf("realtime\n");
#endif

#ifdef CLOCK_PROCESS_CPUTIME_ID
  if(litl_get_time == litl_get_time_process_cputime)
    printf("process_cputime\n");
#endif

#ifdef CLOCK_THREAD_CPUTIME_ID
  if(litl_get_time == litl_get_time_thread_cputime)
    printf("thread_cputime\n");
#endif

#endif	/* CLOCK_GETTIME_AVAIL */

#if defined(__x86_64__) || defined(__i386)
  if(litl_get_time == litl_get_time_ticks)
    printf("ticks\n");
#endif
}

/*
 * Initializes the timing mechanism
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
    } else if (strcmp(time_str, "monotonic") == 0) {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC))
      litl_set_timing_method(litl_get_time_monotonic);
#else
      goto not_available;
#endif
    } else if (strcmp(time_str, "realtime") == 0) {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_REALTIME))
      litl_set_timing_method(litl_get_time_realtime);
#else
      goto not_available;
#endif
    } else if (strcmp(time_str, "process_cputime") == 0) {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_PROCESS_CPUTIME_ID))
      litl_set_timing_method(litl_get_time_process_cputime);
#else
      goto not_available;
#endif
    } else if (strcmp(time_str, "thread_cputime") == 0) {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_THREAD_CPUTIME_ID))
      litl_set_timing_method(litl_get_time_thread_cputime);
#else
      goto not_available;
#endif
    } else if (strcmp(time_str, "ticks") == 0) {
#if defined(__x86_64__) || defined(__i386)
      litl_set_timing_method(litl_get_time_ticks);
#else
      goto not_available;
#endif
    } else if (strcmp(time_str, "none") == 0) {
      litl_set_timing_method(litl_get_time_none);
    } else if (strcmp(time_str, "best") == 0) {
      __litl_time_benchmark();
    } else {
      fprintf(stderr, "Unknown timining method: '%s'\n", time_str);
      abort();
    }
  }
  return;
  not_available: __attribute__ ((__unused__)) fprintf(stderr,
      "Timing function '%s' not available on this system\n", time_str);
  abort();
}

/*
 * Returns -1 if none of timings is available. Otherwise, it returns 0
 */
int litl_set_timing_method(litl_timing_method_t callback) {
  if (!callback)
    return -1;

  litl_get_time = callback;
  return 0;
}

#ifdef CLOCK_GETTIME_AVAIL
static inline litl_time_t __litl_get_time_generic(clockid_t clk_id) {
  litl_time_t time;
  struct timespec tp;
  clock_gettime(clk_id, &tp);
  time = 1000000000 * tp.tv_sec + tp.tv_nsec;
  return time;
}
#endif

/*
 * Uses clock_gettime(CLOCK_MONOTONIC_RAW)
 */
litl_time_t litl_get_time_monotonic_raw() {
#if(defined(CLOCK_GETTIME_AVAIL) && defined( CLOCK_MONOTONIC_RAW))
  return __litl_get_time_generic(CLOCK_MONOTONIC_RAW);
#else
  ERROR_TIMER_NOT_AVAILABLE()
  ;
  return -1;
#endif
}

/*
 * Uses clock_gettime(CLOCK_MONOTONIC)
 */
litl_time_t litl_get_time_monotonic() {
#ifdef CLOCK_GETTIME_AVAIL
  return __litl_get_time_generic(CLOCK_MONOTONIC);
#else
  ERROR_TIMER_NOT_AVAILABLE()
  ;
  return -1;
#endif
}

/*
 * Uses clock_gettime(CLOCK_REALTIME)
 */
litl_time_t litl_get_time_realtime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined (CLOCK_REALTIME))
  return __litl_get_time_generic(CLOCK_REALTIME);
#else
  ERROR_TIMER_NOT_AVAILABLE()
  ;
  return -1;
#endif
}

/*
 * Uses clock_gettime(CLOCK_PROCESS_CPUTIME_ID)
 */
litl_time_t litl_get_time_process_cputime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined (CLOCK_PROCESS_CPUTIME_ID))
  return __litl_get_time_generic(CLOCK_PROCESS_CPUTIME_ID);
#else
  ERROR_TIMER_NOT_AVAILABLE()
  ;
  return -1;
#endif
}

/*
 * Uses clock_gettime(CLOCK_THREAD_CPUTIME_ID)
 */
litl_time_t litl_get_time_thread_cputime() {
#if (defined(CLOCK_GETTIME_AVAIL) && defined(CLOCK_THREAD_CPUTIME_ID))
  return __litl_get_time_generic(CLOCK_THREAD_CPUTIME_ID);
#else
  ERROR_TIMER_NOT_AVAILABLE()
  ;
  return -1;
#endif
}

litl_time_t litl_get_time_none() {
  return 0;
}

/*
 * Uses CPU specific register (for instance, rdtsc for X86* processors)
 */
litl_time_t litl_get_time_ticks() {
#ifdef __x86_64__
  // This is a copy of rdtscll function from asm/msr.h
#define ticks(val) do {						\
    uint32_t __a,__d;						\
    asm volatile("rdtsc" : "=a" (__a), "=d" (__d));		\
    (val) = ((litl_time_t)__a) | (((litl_time_t)__d)<<32);	\
  } while(0)

#elif defined(__i386)

#define ticks(val)				\
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
