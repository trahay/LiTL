/*
 * evnt_types.h
 *
 *  Created on: Oct 30, 2012
 *      Author: Roman Iakymchuk
 */

#ifndef EVNT_TYPES_H_
#define EVNT_TYPES_H_

#include <stdio.h>
#include <stdint.h>

#ifdef USE_GETTID
#include <unistd.h>
#include <sys/syscall.h>  /* For SYS_xxx definitions */
#else
#include <pthread.h>
#endif

/* current thread id */
#ifdef USE_GETTID
#define CUR_TID syscall(SYS_gettid)
#else
#define CUR_TID pthread_self()
#endif

/*
 * Define Evnt-specific data types
 */
typedef uint64_t evnt_tid_t;
typedef uint64_t evnt_time_t;
// TODO: check whether it is possible to use uint32_t for evnt_code_t
typedef uint64_t evnt_code_t;
typedef uint64_t evnt_size_t;
typedef uint64_t evnt_args_t;
typedef uint8_t evnt_data_t;

#define EVNT_TRACE_END 0

#define EVNT_MAX_PARAMS 9

// regular event
typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    // Code contains, in the highest bit, info about raw (1) or regular (0) event
    evnt_code_t code; // code of the event
    evnt_size_t nb_args; // uint8_t number of arguments
    evnt_args_t args[]; // array of arguments; the array is of lengths from 0 to 6
} evnt_t;

// raw event
typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    // Code contains, in the highest bit, info about raw (1) or regular (0) event
    evnt_code_t code; // code of the event
    evnt_size_t size; // size of data in bytes
    evnt_data_t raw[]; // raw data
} evnt_raw_t;

// Flags that corresponds to flushing the buffer
typedef enum {
    EVNT_BUFFER_FLUSH, EVNT_BUFFER_NOFLUSH
} buffer_flags_t;

// Flags that corresponds to the thread-safety
typedef enum {
    EVNT_THREAD_SAFE, EVNT_NOTHREAD_SAFE
} thread_flags_t;

// data structure for holding set of events
typedef uint64_t* trace_t;

// data structure for reading events from trace file
typedef struct {
    FILE* fp;
    trace_t* trace;
    uint32_t offset;
} evnt_trace_t;

#endif /* EVNT_TYPES_H_ */
