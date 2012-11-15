/*
 * timer.c
 *
 * Created by Roman Iakymchuk
 */

#include <stdio.h>
#include <unistd.h>

#include "timer.h"

static long long int ticks_per_sec;

/*
 * This is a copy of rdtscll function from asm/msr.h
 */
#define rdtscll(val) do { \
      unsigned int __a,__d; \
      asm volatile("rdtsc" : "=a" (__a), "=d" (__d)); \
      (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32); \
} while(0)

/*
 * This function initializes the ticks_per_sec variable
 */
void _init_time() __attribute__((constructor));
void _init_time() {
    long long int init_start, init_end;
    rdtscll(init_start);
    usleep(1000000);
    rdtscll(init_end);

    ticks_per_sec = init_end - init_start;
//    printf("1 second is %lld clockticks\n", ticks_per_sec);
//    printf("machine seems to run with %.2lf GHz\n", ticks_per_sec / 1000000000.0);
}

/*
 * This funciton measures time in seconds
 */
double get_time() {
    long long int time;
    rdtscll(time);
    return (double) time / ticks_per_sec;
}

/*
 * This funciton measures time in clock ticks
 */
long long int get_ticks() {
    long long int time;
    rdtscll(time);
    return time;
}

double get_ticks_per_sec() {
    return (double) ticks_per_sec;
}
