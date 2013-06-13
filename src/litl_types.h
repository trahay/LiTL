/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#ifndef LITL_TYPES_H_
#define LITL_TYPES_H_

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
 * Defining LiTL data types
 */
#ifdef __x86_64__
typedef uint64_t litl_tid_t;
typedef uint64_t litl_time_t;
typedef uint32_t litl_code_t;
// TODO: there is a possibility of using uint16_t, however then the alignment would collapse. If this is applied, the
//       function get_event_components() in litl_macro.c should be changed accordingly.
typedef uint64_t litl_trace_size_t;
typedef uint32_t litl_size_t;
typedef uint8_t litl_tiny_size_t;
typedef uint64_t litl_param_t;
typedef uint8_t* litl_buffer_t;// data structure for holding a set of events
typedef uint64_t litl_offset_t;

#elif defined __arm__
typedef uint32_t litl_tid_t;
typedef uint32_t litl_time_t;
typedef uint32_t litl_code_t;
// TODO: there is a possibility of using uint16_t, however then the alignment would collapse. If this is applied, the
//       function get_event_components() in litl_macro.c should be changed accordingly.
typedef uint32_t litl_trace_size_t;
typedef uint32_t litl_size_t;
typedef uint8_t litl_tiny_size_t;
typedef uint32_t litl_param_t;
typedef uint8_t* litl_buffer_t;// data structure for holding a set of events
typedef uint32_t litl_offset_t;
#endif
typedef uint8_t litl_data_t;

#define LITL_OFFSET_CODE 13

#define LITL_MAX_PARAMS 10
#define LITL_MAX_DATA (LITL_MAX_PARAMS * sizeof(litl_param_t))

typedef enum {
    LITL_TYPE_REGULAR, LITL_TYPE_RAW, LITL_TYPE_PACKED, LITL_TYPE_OFFSET
} litl_type_t;

typedef struct {
    litl_time_t time; // time of the measurement
    litl_code_t code; // code of the event
    litl_type_t type;
    union {
        struct {
            litl_tiny_size_t nb_params; // number of arguments
            litl_param_t param[LITL_MAX_PARAMS]; // array of arguments; the array is of lengths from 0 to 10
        }__attribute__((packed)) regular;
        struct {
            litl_size_t size; // size of data in bytes
            litl_data_t data[LITL_MAX_DATA]; // raw data
        }__attribute__((packed)) raw;
        struct {
            litl_tiny_size_t size; // size of data in bytes
            litl_data_t param[LITL_MAX_DATA]; // raw data
        }__attribute__((packed)) packed;
        struct {
            litl_tiny_size_t nb_params; // =1
            litl_param_t offset; // an offset to the next chunk of events
        }__attribute__((packed)) offset;
    } parameters;
}__attribute__((packed)) litl_t;

// data structure that corresponds to the header of a trace file
typedef struct {
    litl_size_t nb_threads;
    litl_tiny_size_t is_trace_archive;
    litl_size_t buffer_size;
    litl_data_t liblitl_ver[8];
    litl_data_t sysinfo[128];
} litl_header_t;

// data structure for pairs (tid, offset) stored in the trace's header
typedef struct {
    litl_tid_t tid;
    litl_offset_t offset;
} litl_header_tids_t;

// data structure for triples (tid, trace_size, offset) stored in the archive's header
typedef struct {
    litl_tid_t fid;
    litl_trace_size_t trace_size;
    litl_offset_t offset;
} litl_header_triples_t;

#define NBBUFFER 1000

/* thread-specific buffer */
typedef struct {
    litl_buffer_t buffer_ptr; /* beginning of the buffer */
    litl_buffer_t buffer_cur; /* pointer to next event */
    litl_tid_t tid; // tid of the working thread
    litl_offset_t offset; // offset to find the next buffer in the trace file

    // used to handle the situation when some threads start after the header was flushed,
    //      i.e. their tids and offsets were not included into the header
    uint8_t already_flushed;
} litl_write_buffer_t;

typedef struct {
    FILE* ftrace;
    char* litl_filename;

    litl_offset_t general_offset; // offset from the beginning of the trace file until the current position

    litl_buffer_t header_ptr;
    litl_buffer_t header_cur;
    litl_size_t header_size;
    litl_size_t header_offset;

    litl_size_t buffer_size; // a buffer size
    uint8_t allow_buffer_flush; // buffer_flush indicates whether buffer flush is enabled (1) or not (0)
    uint8_t is_buffer_full; // in case when the flushing is disabled, the recording of events should be skipped

    pthread_once_t index_once;
    pthread_key_t index; // private thread variable holds its index in tids, buffer_ptr/buffer_cur, and offsets
    litl_size_t nb_threads; // number of threads

    pthread_mutex_t lock_litl_flush; // to handle write conflicts while using pthread
    pthread_mutex_t lock_buffer_init; // to handle race conditions while initializing tids and buffer_ptrs/buffer_curs
    uint8_t allow_thread_safety; // allow_thread_safety indicates whether LiTL uses thread-safety (1) or not (0)

    uint8_t record_tid_activated; // record_tid_activated indicates whether LiTL records tid (1) or not (0)
    // litl_initialized is used to ensure that EZTrace does not start recording events before the initialization is
    //      finished
    uint8_t litl_initialized;
    // litl_paused indicates whether LiTL stops recording events (1) for a while or not (0)
    volatile uint8_t litl_paused;

    // is_header_flushed is used to check whether the header with tids and offsets has been flushed
    uint8_t is_header_flushed;

    /* array of thread-specific buffers */
    litl_write_buffer_t buffers[NBBUFFER];
} litl_trace_write_t;

#define GET_CUR_EVENT_PER_THREAD(_trace_, _thread_index_) (&(_trace_)->buffers[(_thread_index_)].cur_event)
#define GET_CUR_EVENT(_trace_) GET_CUR_EVENT_PER_THREAD(_trace_, (trace)->cur_index)

#define LITL_GET_TID(_read_event_) (_read_event_)->tid
#define LITL_GET_TIME(_read_event_) (_read_event_)->event->time
#define LITL_GET_TYPE(_read_event_) (_read_event_)->event->type
#define LITL_GET_CODE(_read_event_) (_read_event_)->event->code

#define LITL_RAW(_read_event_) (&(_read_event_)->event->parameters.raw)
#define LITL_REGULAR(_read_event_) (&(_read_event_)->event->parameters.regular)
#define LITL_PACKED(_read_event_) (&(_read_event_)->event->parameters.packed)
#define LITL_OFFSET(_read_event_) (&(_read_event_)->event->parameters.offset)

typedef struct {
    litl_tid_t tid; /* thread id */
    litl_t *event; /* pointer to the event */
} litl_read_t;

typedef struct {
    litl_header_tids_t* tids;

    litl_buffer_t buffer_ptr; // pointer to the beginning of the buffer
    litl_buffer_t buffer; // pointer to the current position in the buffer
    litl_size_t buffer_size;

    litl_offset_t offset; // offset from the beginning of the buffer
    litl_offset_t tracker; // indicator of the end of the buffer = offset + buffer_size

    litl_offset_t file_offset; // offset from the beginning of the file

    litl_read_t cur_event; /* current event */

} litl_trace_read_thread_t;

// data structure for reading events from a trace file
typedef struct {
    FILE* fp; // pointer to the trace file

    litl_buffer_t header_buffer_ptr;
    litl_buffer_t header_buffer;
    litl_size_t header_size;
    litl_header_t* header;
    litl_size_t buffer_size;

    litl_size_t nb_buffers;
    litl_trace_read_thread_t *buffers;
    int cur_index; /* index of the thread of the current event */
    int initialized; /* set to 1 when initialized */

} litl_trace_read_t;

// data structure for merging traces in the archive
typedef struct {
    int f_arch;

    litl_buffer_t buffer;
    litl_size_t buffer_size;

    litl_offset_t header_offset; // offset from the beginning of the trace header
    litl_offset_t general_offset; // offset from the beginning of the trace file until the current position
} litl_trace_merge_t;

// data structure for splitting an archive of traces
typedef struct {
    int f_arch;
    //    char *f_arch_name;

    litl_buffer_t header_buffer;
    litl_size_t header_size;
    litl_header_t *header;
    litl_size_t nb_traces;
    litl_header_triples_t* triples;

    litl_buffer_t buffer;
    litl_size_t buffer_size;
} litl_trace_split_t;

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

#endif /* LITL_TYPES_H_ */
