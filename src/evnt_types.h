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

#define EVNT_TRACE_START 0
#define EVNT_BUFFER_FLUSHED 1
#define EVNT_TRACE_END 2

#define EVNT_MAX_PARAMS 9

typedef struct {
    uint64_t tid; // thread ID
    uint64_t time; // time of the measurement
    uint64_t code; // code of the event
    uint64_t nb_args; // number of arguments
    uint64_t args[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 6
} evnt;

typedef struct {
    uint64_t tid; // thread ID
    uint64_t time; // time of the measurement
    uint64_t code; // code of the event
    uint64_t size; // size of data
    void* data; // data contains an array of arguments
} evnt_raw;

// Flags that corresponds to flushing the buffer
typedef enum {
    EVNT_BUFFER_FLUSH, EVNT_BUFFER_NOFLUSH
} buffer_flags;

// Flags that corresponds to the thread-safety
typedef enum {
    EVNT_THREAD_SAFE, EVNT_NOTHREAD_SAFE
} thread_flags;

typedef uint64_t Trace;

typedef struct {
    FILE* fp;
    char* filename; // can be unnecessary
    evnt * cur_evnt;
} read_trace;

#endif /* EVNT_TYPES_H_ */
