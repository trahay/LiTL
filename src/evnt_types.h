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
typedef uint64_t evnt_code_t;
typedef uint64_t evnt_size_t;
typedef uint64_t evnt_args_t;
typedef uint64_t evnt_data_t;

#define EVNT_TRACE_END 0

#define EVNT_MAX_PARAMS 9
#define EVNT_MAX_DATA (EVNT_MAX_PARAMS * sizeof(uint64_t))

typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    // TODO: code should contain in the highest bit info about raw (1) or plain (0) event
    evnt_code_t code; // code of the event
    evnt_size_t nb_args; // uint16_t number of arguments or size of raw data
    evnt_args_t args[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to EVNT_MAX_PARAMS
    evnt_data_t raw[EVNT_MAX_DATA]; // raw data
} evnt_gen;

typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    evnt_code_t code; // code of the event
    evnt_size_t nb_args; // uint8_t number of arguments
    evnt_args_t args[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 6
} evnt;

typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    evnt_code_t code; // code of the event
    evnt_size_t size; // size of data in bytes
    evnt_args_t raw[EVNT_MAX_DATA]; // raw data
} evnt_raw;

// Flags that corresponds to flushing the buffer
typedef enum {
    EVNT_BUFFER_FLUSH, EVNT_BUFFER_NOFLUSH
} buffer_flags;

// Flags that corresponds to the thread-safety
typedef enum {
    EVNT_THREAD_SAFE, EVNT_NOTHREAD_SAFE
} thread_flags;

typedef uint64_t* trace;

typedef struct {
    FILE* fp;
    char* filename; // can be unnecessary
    evnt * cur_evnt;
} read_trace;

#endif /* EVNT_TYPES_H_ */
