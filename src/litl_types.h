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
 * LiTL data types
 */
#ifdef __x86_64__
typedef uint64_t litl_tid_t;
typedef uint64_t litl_time_t;
// TODO: there is a possibility of using uint16_t, however then the alignment
//       would collapse
typedef uint32_t litl_code_t;
typedef uint64_t litl_trace_size_t;
typedef uint8_t litl_tiny_size_t;
typedef uint16_t litl_med_size_t;
typedef uint32_t litl_size_t;
typedef uint64_t litl_param_t;
typedef uint8_t* litl_buffer_t; // data structure for holding a set of events
typedef uint64_t litl_offset_t;

#elif defined __arm__
typedef uint32_t litl_tid_t;
typedef uint32_t litl_time_t;
typedef uint32_t litl_code_t;
// TODO: there is a possibility of using uint16_t, however then the alignment
//       would collapse
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

typedef enum
    __attribute__((packed)) {
        LITL_TYPE_REGULAR, LITL_TYPE_RAW, LITL_TYPE_PACKED, LITL_TYPE_OFFSET
} litl_type_t;

typedef struct {
    litl_time_t time; // time of the measurement
    litl_code_t code; // code of the event
    litl_type_t type;
    union {
        struct {
            litl_tiny_size_t nb_params; // number of arguments
            // array of arguments; the array is of lengths from 0 to 10
            litl_param_t param[LITL_MAX_PARAMS];
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

// is used to store threads info (tids pairs) in one data slot
#define NBTHREADS 32

/*
 * Data structure that corresponds to the header of a trace file
 */
typedef struct {
    litl_med_size_t nb_threads; // the total number of threads
    litl_tiny_size_t is_trace_archive;
    litl_size_t buffer_size;
    litl_med_size_t header_nb_threads; // the number of threads in the header
    litl_data_t litl_ver[8];
    litl_data_t sysinfo[128];
}__attribute__((packed)) litl_header_t;

/*
 * Data structure for pairs (tid, offset) stored in the trace's header
 */
typedef struct {
    litl_tid_t tid;
    litl_offset_t offset;
} litl_header_tids_t;

/*
 * Data structure for triples (tid, trace_size, offset) stored in the archive's
 *     header
 */
typedef struct {
    litl_med_size_t fid;
    litl_trace_size_t trace_size;
    litl_offset_t offset;
}__attribute__((packed)) litl_header_triples_t;

/*
 * Thread-specific buffer
 */
typedef struct {
    litl_buffer_t buffer_ptr; /* beginning of the buffer */
    litl_buffer_t buffer_cur; /* pointer to next event */
    litl_tid_t tid; // tid of the working thread
    litl_offset_t offset; // offset to find the next buffer in the trace file

    // used to handle the situation when some threads start after the header was
    //      flushed, i.e. their tids and offsets were not included into the header
    uint8_t already_flushed;
} litl_write_buffer_t;

typedef struct {
    int ftrace;
    char* filename;

    litl_offset_t general_offset; // offset from the beginning of the trace file

    litl_buffer_t header_ptr;
    litl_buffer_t header_cur;
    litl_size_t header_size;
    litl_size_t header_offset;

    // the number of slots with the information on threads; first slot, which is
    //    in the header, does not count; each contains at most NBTHREADS threads
    litl_med_size_t nb_slots;
    litl_med_size_t header_nb_threads; // the number of threads in the header
    // an offset to the next chunk of pairs (tid, offset)
    litl_param_t threads_offset;
    // is_header_flushed is used to check whether the header with tids and
    //    offsets has been flushed
    uint8_t is_header_flushed;

    litl_size_t buffer_size; // a buffer size
    // buffer_flush indicates whether buffer flush is enabled (1) or not (0)
    uint8_t allow_buffer_flush;
    // in case when the flushing is disabled, the recording of events should be
    //     skipped
    uint8_t is_buffer_full;

    pthread_once_t index_once;
    // private thread variable holds its index in tids, buffer_ptr/buffer_cur,
    //     and offsets
    pthread_key_t index;
    litl_med_size_t nb_threads; // number of threads

    // to handle write conflicts while using pthread
    pthread_mutex_t lock_litl_flush;
    // to handle race conditions while initializing tids and buffer_ptrs/buffer_curs
    pthread_mutex_t lock_buffer_init;
    // allow_thread_safety indicates whether LiTL uses thread-safety (1) or not (0)
    uint8_t allow_thread_safety;

    // record_tid_activated indicates whether LiTL records tid (1) or not (0)
    uint8_t record_tid_activated;
    // litl_initialized is used to ensure that EZTrace does not start recording
    //     events before the initialization is finished
    uint8_t litl_initialized;
    // litl_paused indicates whether LiTL stops recording events (1) for a while
    //     or not (0)
    volatile uint8_t litl_paused;

    litl_write_buffer_t **buffers; // array of thread-specific buffers
    // number of thread-specific buffers that are allocated
    unsigned nb_allocated_buffers;

} litl_trace_write_t;

typedef struct {
    litl_tid_t tid; // thread id
    litl_t *event; // pointer to the event
} litl_read_t;

/*
 * Data structure for reading thread related events
 */
typedef struct {
    litl_header_tids_t* tids;

    litl_buffer_t buffer; // pointer to the current position in the buffer
    litl_buffer_t buffer_ptr; // pointer to the beginning of the buffer
    litl_size_t buffer_size;

    litl_offset_t offset; // offset from the beginning of the buffer
    // indicator of the end of the buffer = offset + buffer_size
    litl_offset_t tracker;

    litl_read_t cur_event; // current event

} litl_trace_read_thread_t;

/*
 * Data structure for reading process related events
 */
typedef struct {
    litl_header_triples_t* triples;

    litl_size_t header_size;
    litl_header_t* header;
    litl_buffer_t header_buffer;
    litl_buffer_t header_buffer_ptr;

    litl_med_size_t nb_buffers;
    litl_size_t buffer_size;
    litl_trace_read_thread_t *buffers;

    int cur_index; // index of the thread of the current event
    int initialized; // set to 1 when initialized

} litl_trace_read_process_t;

/*
 * Data structure for reading events from either a trace or an archive of traces
 */
typedef struct {
    int f_handle;

    litl_size_t header_size;
    litl_buffer_t header_buffer;

    // indicates whether a trace is:
    //   (=0) a regular trace;
    //   (=1) an archive of traces;
    //   (=2) an archive of archives
    litl_tiny_size_t is_trace_archive;
    litl_med_size_t nb_traces;
    litl_trace_read_process_t *traces;
} litl_trace_read_t;

/*
 * Data structure for merging traces in the archive
 */
typedef struct {
    int f_arch;

    litl_buffer_t buffer;
    litl_size_t buffer_size;

    // offset from the beginning of the trace header
    litl_offset_t header_offset;
    // offset from the beginning of the trace file until the current position
    litl_offset_t general_offset;
} litl_trace_merge_t;

/*
 * Data structure for splitting an archive of traces
 */
typedef struct {
    int f_arch;

    litl_size_t header_size;
    litl_buffer_t header_buffer;

    litl_tiny_size_t is_trace_archive;
    litl_med_size_t nb_traces;
    litl_header_triples_t* triples;

    litl_buffer_t buffer;
    litl_size_t buffer_size;
} litl_trace_split_t;

/*
 * Defining formats for printing data
 */
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


/*
 * For internal use only.
 * Computes the offset of MEMBER in structure TYPE
 */
#define __litl_offset_of(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

/*
 * For internal use only.
 * Computes the offset of the first parameter in an event
 */
#define LITL_BASE_SIZE __litl_offset_of(litl_t, parameters.regular.param)


#endif /* LITL_TYPES_H_ */
