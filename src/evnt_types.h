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
#ifdef __x86_64__
typedef uint64_t evnt_tid_t;
typedef uint64_t evnt_time_t;
typedef uint32_t evnt_code_t;
// TODO: there is a possibility of using uint16_t, however then the alignment would collapse. If this is applied, the
//       function get_event_components() in evnt_macro.c should be changed accordingly.
typedef uint64_t evnt_file_size_t;
typedef uint32_t evnt_size_t;
typedef uint8_t evnt_tiny_size_t;
typedef uint64_t evnt_param_t;
typedef uint8_t* evnt_buffer_t; // data structure for holding a set of events
typedef uint64_t evnt_offset_t;

#elif defined __arm__
typedef uint32_t evnt_tid_t;
typedef uint32_t evnt_time_t;
typedef uint32_t evnt_code_t;
// TODO: there is a possibility of using uint16_t, however then the alignment would collapse. If this is applied, the
//       function get_event_components() in evnt_macro.c should be changed accordingly.
typedef uint32_t evnt_file_size_t;
typedef uint32_t evnt_size_t;
typedef uint8_t evnt_tiny_size_t;
typedef uint32_t evnt_param_t;
typedef uint8_t* evnt_buffer_t;// data structure for holding a set of events
typedef uint32_t evnt_offset_t;
#endif
typedef uint8_t evnt_data_t;

#define EVNT_OFFSET_CODE 13

#define EVNT_MAX_PARAMS 10
#define EVNT_MAX_DATA (EVNT_MAX_PARAMS * sizeof(evnt_param_t))

typedef enum {
    EVNT_TYPE_REGULAR, EVNT_TYPE_RAW, EVNT_TYPE_PACKED, EVNT_TYPE_OFFSET
} evnt_type_t;

typedef struct {
    evnt_time_t time; // time of the measurement
    evnt_code_t code; // code of the event
    evnt_type_t type;
    union {
        struct {
            evnt_tiny_size_t nb_params; // number of arguments
            evnt_param_t param[EVNT_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 10
        }__attribute__((packed)) regular;
        struct {
            evnt_size_t size; // size of data in bytes
            evnt_data_t data[EVNT_MAX_DATA]; // raw data
        }__attribute__((packed)) raw;
        struct {
            evnt_tiny_size_t size; // size of data in bytes
            evnt_data_t param[EVNT_MAX_DATA]; // raw data
        }__attribute__((packed)) packed;
        struct {
            evnt_tiny_size_t nb_params; // =1
            evnt_param_t offset; // an offset to the next chunk of events
        }__attribute__((packed)) offset;
    } parameters;
}__attribute__((packed)) evnt_t;

// data structure for reading events from a trace file
typedef struct {
    FILE* fp; // pointer to the trace file
    evnt_buffer_t buffer_ptr; // pointer to the beginning of the buffer
    evnt_buffer_t buffer; // pointer either to the next event or to the end of the trace
    uint32_t offset; // offset from the beginning of the trace file
    uint32_t tracker; // indicator of the end of the buffer = offset + buffer size
} evnt_block_t;

// data structure that corresponds to the header of a trace file
typedef struct {
    evnt_data_t libevnt_ver[8];
    evnt_data_t sysinfo[128];
    evnt_size_t nb_threads;
    evnt_size_t buffer_size;
} evnt_header_t;

// data structure for the pairs (tid, offset) stored in the trace header
typedef struct {
    evnt_tid_t tid;
    evnt_offset_t offset;
} evnt_header_tids_t;

#define NBBUFFER 1000

/* thread-specific buffer */
typedef struct{
    evnt_buffer_t buffer_ptr;	/* beginning of the buffer */
    evnt_buffer_t buffer_cur;	/* pointer to next event */
    evnt_tid_t tid; // tid of the working thread
    evnt_offset_t offset; // offset to find the next buffer in the trace file

    // used to handle the situation when some threads start after the header was flushed,
    //      i.e. their tids and offsets were not included into the header
    uint8_t already_flushed;
} write_buffer_t;

typedef struct {
    FILE* ftrace;
    char* evnt_filename;

    evnt_offset_t general_offset; // offset from the beginning of the trace file until the current position

    evnt_buffer_t header_ptr;
    evnt_buffer_t header_cur;
    evnt_size_t header_size;
    evnt_size_t header_offset;

    evnt_size_t buffer_size; // a buffer size
    uint8_t allow_buffer_flush; // buffer_flush indicates whether buffer flush is enabled (1) or not (0)
    uint8_t is_buffer_full; // in case when the flushing is disabled, the recording of events should be skipped

    pthread_once_t index_once;
    pthread_key_t index; // private thread variable holds its index in tids, buffer_ptr/buffer_cur, and offsets
    evnt_size_t nb_threads; // number of threads

    pthread_mutex_t lock_evnt_flush; // to handle write conflicts while using pthread
    pthread_mutex_t lock_buffer_init; // to handle race conditions while initializing tids and buffer_ptrs/buffer_curs
    uint8_t allow_thread_safety; // allow_thread_safety indicates whether libevnt uses thread-safety (1) or not (0)

    uint8_t record_tid_activated; // record_tid_activated indicates whether libevnt records tid (1) or not (0)
    // evnt_initialized is used to ensure that EZTrace does not start recording events before the initialization is
    //      finished
    uint8_t evnt_initialized;
    // evnt_paused indicates whether libevnt stops recording events (1) for a while or not (0)
    volatile uint8_t evnt_paused;

    // is_header_flushed is used to check whether the header with tids and offsets has been flushed
    uint8_t is_header_flushed;

    /* array of thread-specific buffers */
    write_buffer_t buffers[NBBUFFER];
} evnt_trace_write_t;

#define GET_CUR_EVENT_PER_THREAD(_trace_, _thread_index_) (&(_trace_)->buffers[(_thread_index_)].cur_event)
#define GET_CUR_EVENT(_trace_) GET_CUR_EVENT_PER_THREAD(_trace_, (trace)->cur_index)

#define EVNT_GET_TID(_read_event_) (_read_event_)->tid
#define EVNT_GET_TIME(_read_event_) (_read_event_)->event->time
#define EVNT_GET_TYPE(_read_event_) (_read_event_)->event->type
#define EVNT_GET_CODE(_read_event_) (_read_event_)->event->code

#define EVNT_RAW(_read_event_) (&(_read_event_)->event->parameters.raw)
#define EVNT_REGULAR(_read_event_) (&(_read_event_)->event->parameters.regular)
#define EVNT_PACKED(_read_event_) (&(_read_event_)->event->parameters.packed)
#define EVNT_OFFSET(_read_event_) (&(_read_event_)->event->parameters.offset)

typedef struct {
    evnt_tid_t tid; /* thread id */
    evnt_t *event; /* pointer to the event */
} evnt_read_t;

typedef struct {
    evnt_header_tids_t* tids;

    evnt_buffer_t buffer_ptr; // pointer to the beginning of the buffer
    evnt_buffer_t buffer; // pointer to the current position in the buffer
    evnt_size_t buffer_size;

    evnt_offset_t offset; // offset from the beginning of the buffer
    evnt_offset_t tracker; // indicator of the end of the buffer = offset + buffer_size

    evnt_offset_t file_offset; // offset from the beginning of the file

    evnt_read_t cur_event; /* current event */

} evnt_trace_read_thread_t;

// data structure for reading events from a trace file
typedef struct {
    FILE* fp; // pointer to the trace file

    evnt_buffer_t header_buffer_ptr;
    evnt_buffer_t header_buffer;
    evnt_size_t header_size;
    evnt_header_t* header;
    evnt_size_t buffer_size;

    evnt_size_t nb_buffers;
    evnt_trace_read_thread_t *buffers;
    int cur_index; /* index of the thread of the current event */
    int initialized; /* set to 1 when initialized */

} evnt_trace_read_t;

// defining the printing formats
#ifdef __x86_64__
#define PRTIu32 "u"
#define PRTIu64 "lu"
#define PRTIx32 "x"
#define PRTIx64 "lx"
#elif defined __arm__
#define PRTIu32 "u"
#define PRTIu64 "u"
#define PRTIx32 "x"
#define PRTIx64 "x"
#endif

#endif /* EVNT_TYPES_H_ */
