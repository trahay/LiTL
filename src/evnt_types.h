/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef EVNT_TYPES_H_
#define EVNT_TYPES_H_

#include <stdio.h>
#include <stdint.h>

#ifdef USE_GETTID
#include <unistd.h>
#include <sys/syscall.h>  // For SYS_xxx definitions
#else
#include <pthread.h>
#endif

// current thread id
#ifdef USE_GETTID
#define CUR_TID syscall(SYS_gettid)
#else
#define CUR_TID pthread_self()
#endif

/*
 * Defining libevnt data types
 */
typedef uint64_t evnt_tid_t;
typedef uint64_t evnt_time_t;
// TODO: check whether it is possible to use uint32_t for evnt_code_t
typedef uint64_t evnt_code_t;
// TODO: change to uint8_t
typedef uint64_t evnt_size_t;
typedef uint64_t evnt_param_t;
typedef uint8_t evnt_data_t;

#define EVNT_TRACE_END 0

#define EVNT_MAX_PARAMS 10
#define EVNT_MAX_DATA (EVNT_MAX_PARAMS * sizeof(uint64_t))

// regular event
typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    evnt_code_t code; // code of the event. Code contains, in the highest bit, info about raw (1) or regular (0) event
    evnt_size_t nb_params; // number of arguments
    evnt_param_t param[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 10
} evnt_t;

// raw event
typedef struct {
    evnt_tid_t tid; // thread ID
    evnt_time_t time; // time of the measurement
    evnt_code_t code; // code of the event. Code contains, in the highest bit, info about raw (1) or regular (0) event
    evnt_size_t size; // size of data in bytes
    evnt_data_t raw[EVNT_MAX_DATA]; // raw data
} evnt_raw_t;

// data structure for holding a set of events
typedef uint64_t* evnt_trace_t;

// data structure for reading events from trace file
typedef struct {
    FILE* fp; // pointer to the trace file
    evnt_trace_t trace_head; // pointer to the beginning of the buffer
    evnt_trace_t trace; // pointer either to the next event or to the end of the trace
    uint32_t offset; // offset from the beginning of the trace file
    uint32_t tracker; // indicator of the end of the buffer = offset + buffer size
} evnt_block_t;

#endif /* EVNT_TYPES_H_ */
