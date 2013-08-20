/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>

#include "litl_timer.h"
#include "litl_macro.h"
#include "litl_write.h"

/*
 * Adds a header to the trace file with the information regarding:
 *   - OS
 *   - Processor type
 *   - Version of LiTL
 */
static void __litl_write_add_trace_header(litl_trace_write_t* trace) {
    struct utsname uts;

    // allocate memory for the trace header
    trace->header_ptr = (litl_buffer_t) malloc(trace->header_size);
    if (!trace->header_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header_cur = trace->header_ptr;

    if (uname(&uts) < 0)
        perror("Could not use uname()!");

    // get the number of symbols for litl_ver
    sprintf((char*) ((litl_header_t *) trace->header_cur)->litl_ver, "%s",
            VERSION);

    // get the number of symbols for sysinfo
    sprintf((char*) ((litl_header_t *) trace->header_cur)->sysinfo,
            "%s %s %s %s %s", uts.sysname, uts.nodename, uts.release,
            uts.version, uts.machine);

    // add nb_threads and buffer_size
    ((litl_header_t *) trace->header_cur)->nb_threads = trace->nb_threads;
    // by default a trace file per process is recorded
    ((litl_header_t *) trace->header_cur)->is_trace_archive = 0;
    ((litl_header_t *) trace->header_cur)->header_nb_threads =
            trace->nb_threads;
    ((litl_header_t *) trace->header_cur)->buffer_size = trace->buffer_size;

    // header_size stores the position of nb_threads in the trace file
    trace->header_size = 0;

    // size of two strings (LiTL, OS), nb_threads, and buffer_size
    trace->header_cur += sizeof(litl_header_t);
}

/*
 * To create trace->buffer_ptr and trace->buffer_cur
 */
static void __litl_write_init_var(litl_trace_write_t* trace) {
    pthread_key_create(&trace->index, NULL );
}

/*
 * Initializes the trace buffer
 */
litl_trace_write_t litl_write_init_trace(const litl_size_t buf_size) {
    litl_med_size_t i;
    litl_trace_write_t trace;

    trace.buffer_size = buf_size;

    // set variables
    trace.filename = NULL;
    trace.is_buffer_full = 0;
    trace.is_header_flushed = 0;
    litl_write_tid_recording_on(&trace);

    trace.nb_allocated_buffers = 256;
    trace.buffers = realloc(NULL,
            sizeof(litl_write_buffer_t*) * trace.nb_allocated_buffers);

    for (i = 0; i < trace.nb_allocated_buffers; i++) {
        // initialize the array already_flushed
        trace.buffers[i] = malloc(sizeof(litl_write_buffer_t));
        trace.buffers[i]->already_flushed = 0;

        // initialize tids by zeros; this is needed for __is_tid and __find_slot
        trace.buffers[i]->tid = 0;
    }
    trace.nb_threads = 0;

    // initialize the timing mechanism
    litl_time_initialize();

    // a jump function is needed 'cause it is not possible to pass args to
    //   the calling function through pthread_once
    void __init() {
        __litl_write_init_var(&trace);
    }
    trace.index_once = PTHREAD_ONCE_INIT;
    pthread_once(&trace.index_once, __init);

    // set trace.allow_buffer_flush using the environment variable.
    //   By default the buffer flushing is enabled
    char* str = getenv("LITL_BUFFER_FLUSH");
    if (str && (strcmp(str, "off") == 0))
        litl_write_buffer_flush_off(&trace);
    else
        litl_write_buffer_flush_on(&trace);

    // set trace.allow_thread_safety using the environment variable.
    //   By default thread safety is enabled
    str = getenv("LITL_THREAD_SAFETY");
    if (str && (strcmp(str, "off") == 0))
        litl_write_thread_safety_off(&trace);
    else
        litl_write_thread_safety_on(&trace);

    if (trace.allow_thread_safety)
        pthread_mutex_init(&trace.lock_litl_flush, NULL );
    pthread_mutex_init(&trace.lock_buffer_init, NULL );

    // TODO: touch each block in buffer_ptr in order to load it

    trace.litl_paused = 0;
    trace.litl_initialized = 1;

    return trace;
}

/*
 * Computes the size of data in the trace header
 */
static litl_size_t __litl_write_get_header_size(litl_trace_write_t* trace) {
    return (trace->header_cur - trace->header_ptr);
}

/*
 * Computes the size of data in buffer
 */
static litl_size_t __litl_write_get_buffer_size(litl_trace_write_t* trace,
        litl_med_size_t pos) {
    return (trace->buffers[pos]->buffer_cur - trace->buffers[pos]->buffer_ptr);
}

/*
 * Activates buffer flush
 */
void litl_write_buffer_flush_on(litl_trace_write_t* trace) {
    trace->allow_buffer_flush = 1;
}

/*
 * Deactivates buffer flush. By default, it is activated
 */
void litl_write_buffer_flush_off(litl_trace_write_t* trace) {
    trace->allow_buffer_flush = 0;
}

/*
 * Activate thread safety. By default it is deactivated
 */
void litl_write_thread_safety_on(litl_trace_write_t* trace) {
    trace->allow_thread_safety = 1;
}

/*
 * Deactivates thread safety
 */
void litl_write_thread_safety_off(litl_trace_write_t* trace) {
    trace->allow_thread_safety = 0;
}

/*
 * Activates recording tid. By default it is deactivated
 */
void litl_write_tid_recording_on(litl_trace_write_t* trace) {
    trace->record_tid_activated = 1;
}

/*
 * Deactivates recording tid
 */
void litl_write_tid_recording_off(litl_trace_write_t* trace) {
    trace->record_tid_activated = 0;
}

/*
 * Pauses the event recording
 */
void litl_write_pause_recording(litl_trace_write_t* trace) {
    if (trace)
        trace->litl_paused = 1;
}

/*
 * Resumes the event recording
 */
void litl_write_resume_recording(litl_trace_write_t* trace) {
    if (trace)
        trace->litl_paused = 0;
}

/*
 * Sets a new name for the trace file
 */
void litl_write_set_filename(litl_trace_write_t* trace, char* filename) {
    if (trace->filename) {
        if (trace->is_header_flushed)
            fprintf(stderr,
                    "Warning: changing the trace file name to %s after some events have been saved in file %s\n",
                    filename, trace->filename);
        free(trace->filename);
    }

    // check whether the file name was set. If no, set it by default trace name.
    if (filename == NULL )
        sprintf(filename, "/tmp/%s_%s", getenv("USER"), "litl_log_1");

    if (asprintf(&trace->filename, "%s", filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Writes the recorded events from the buffer to the trace file
 */
void litl_write_flush_buffer(litl_trace_write_t* trace, litl_med_size_t index) {
    int res __attribute__ ((__unused__));

    if (!trace->litl_initialized)
        return;

    if (trace->allow_thread_safety)
        pthread_mutex_lock(&trace->lock_litl_flush);

    if (!trace->is_header_flushed) {
        // check whether the trace file can be opened
        if ((trace->f_handle = open(trace->filename, O_WRONLY | O_CREAT, 0644))
                < 0) {
            fprintf(stderr, "Cannot open %s\n", trace->filename);
            exit(EXIT_FAILURE);
        }

        // add a header to the trace file
        trace->header_size = sizeof(litl_header_t)
                + (trace->nb_threads + 1) * sizeof(litl_header_tids_t);
        __litl_write_add_trace_header(trace);

        // add information about each working thread: (tid, offset)
        litl_med_size_t i;
        for (i = 0; i < trace->nb_threads; i++) {
            ((litl_header_tids_t *) trace->header_cur)->tid =
                    trace->buffers[i]->tid;
            ((litl_header_tids_t *) trace->header_cur)->offset = 0;

            trace->header_cur += sizeof(litl_header_tids_t);

            // save the position of offset inside the trace file
            trace->buffers[i]->offset = __litl_write_get_header_size(trace)
                    - sizeof(litl_offset_t);
            trace->buffers[i]->already_flushed = 1;
        }

        // offset indicates the position of offset to the next slot of
        //   pairs (tid, offset) within the trace file
        trace->header_offset = __litl_write_get_header_size(trace);

        // specify the last slot of pairs (offset == 0)
        ((litl_header_tids_t *) trace->header_cur)->tid = 0;
        ((litl_header_tids_t *) trace->header_cur)->offset = 0;
        trace->header_cur += sizeof(litl_header_tids_t);

        // write the trace header to the trace file
        if (write(trace->f_handle, trace->header_ptr,
                __litl_write_get_header_size(trace)) == -1) {
            perror(
                    "Flushing the buffer. Could not write measured data to the trace file!");
            exit(EXIT_FAILURE);
        }

        // set the general_offset
        trace->general_offset = __litl_write_get_header_size(trace);

        trace->header_nb_threads = trace->nb_threads;
        trace->threads_offset = 0;
        trace->nb_slots = 0;

        trace->is_header_flushed = 1;
    }

    // handle the situation when some threads start after the header was flushed
    if (!trace->buffers[index]->already_flushed) {

        // when more buffers to store threads information is required
        if (trace->nb_threads
                > (trace->header_nb_threads + NBTHREADS * trace->nb_slots)) {

            // updated the offset from the previous slot
            lseek(trace->f_handle, trace->header_offset + sizeof(litl_tid_t),
                    SEEK_SET);
            res = write(trace->f_handle, &trace->general_offset,
                    sizeof(litl_offset_t));

            // reserve a new slot for pairs (tid, offset)
            trace->header_offset = trace->general_offset;
            trace->threads_offset = trace->header_offset;
            trace->general_offset += (NBTHREADS + 1)
                    * sizeof(litl_header_tids_t);

            trace->nb_slots++;
        }

        // add a new pair (tid, offset)
        lseek(trace->f_handle, trace->header_offset, SEEK_SET);
        res = write(trace->f_handle, &trace->buffers[index]->tid,
                sizeof(litl_tid_t));
        res = write(trace->f_handle, &trace->general_offset,
                sizeof(litl_offset_t));

        // add an indicator to specify the last slot of pairs (offset == 0)
        // TODO: how to optimize this and write only once at the end of the slot
        res = write(trace->f_handle, &trace->buffers[index]->already_flushed,
                sizeof(litl_tid_t));
        res = write(trace->f_handle, &trace->buffers[index]->already_flushed,
                sizeof(litl_offset_t));

        trace->header_offset += sizeof(litl_header_tids_t);
        trace->buffers[index]->already_flushed = 1;

        // updated the number of threads
        // TODO: perform update only once 'cause there is duplication
        //        printf("trace->nb_threads = %d\n", trace->nb_threads);
        lseek(trace->f_handle, trace->header_size, SEEK_SET);
        res = write(trace->f_handle, &trace->nb_threads,
                sizeof(litl_med_size_t));
        lseek(trace->f_handle, trace->general_offset, SEEK_SET);
    } else {
        // update the previous offset of the current thread,
        //   updating the location in the file
        lseek(trace->f_handle, trace->buffers[index]->offset, SEEK_SET);
        res = write(trace->f_handle, &trace->general_offset,
                sizeof(litl_offset_t));
        lseek(trace->f_handle, trace->general_offset, SEEK_SET);
    }

    // add an event with offset
    litl_write_probe_offset(trace, index);
    if (write(trace->f_handle, trace->buffers[index]->buffer_ptr,
            __litl_write_get_buffer_size(trace, index)) == -1) {
        perror(
                "Flushing the buffer. Could not write measured data to the trace file!");
        abort();
    }

    // update the general_offset
    trace->general_offset += __litl_write_get_buffer_size(trace, index);
    // update the current offset of the thread
    trace->buffers[index]->offset = trace->general_offset
            - sizeof(litl_offset_t);

    if (trace->allow_thread_safety)
        pthread_mutex_unlock(&trace->lock_litl_flush);

    trace->buffers[index]->buffer_cur = trace->buffers[index]->buffer_ptr;
}

/*
 * Checks whether the trace buffer was allocated. If no, then allocate
 *    the buffer and, for otherwise too, returns the position of
 *    the thread buffer in the array buffer_ptr/buffer_cur.
 */
static void __litl_write_allocate_buffer(litl_trace_write_t* trace) {
    litl_med_size_t* pos;

    // thread safe region
    pthread_mutex_lock(&trace->lock_buffer_init);

    pos = malloc(sizeof(litl_med_size_t));
    *pos = trace->nb_threads;
    pthread_setspecific(trace->index, pos);
    trace->nb_threads++;

    if (*pos >= trace->nb_allocated_buffers) {
        // We need to allocate a bigger array of buffers
        void* ptr = realloc(trace->buffers,
                trace->nb_allocated_buffers * 2 * sizeof(litl_write_buffer_t*));
        if (!ptr) {
            fprintf(stderr, "LiTL failed to allocate memory. Aborting.\n");
            abort();
        }

        trace->buffers = ptr;
        unsigned i;
        for (i = trace->nb_allocated_buffers;
                i < 2 * trace->nb_allocated_buffers; i++) {
            trace->buffers[i] = malloc(sizeof(litl_write_buffer_t));
            trace->buffers[i]->already_flushed = 0;
        }
        trace->nb_allocated_buffers *= 2;
    }

    trace->buffers[*pos]->tid = CUR_TID;
    trace->buffers[*pos]->already_flushed = 0;

    pthread_mutex_unlock(&trace->lock_buffer_init);

    trace->buffers[*pos]->buffer_ptr = malloc(
            trace->buffer_size + litl_get_reg_event_size(LITL_MAX_PARAMS)
                    + litl_get_reg_event_size(1));
    if (!trace->buffers[*pos]->buffer_ptr) {
        perror("Could not allocate memory for the buffer!");
        exit(EXIT_FAILURE);
    }

    // touch the memory so that it is allocated for real (otherwise, this may
    //    cause performance issues on NUMA machines)
    memset(trace->buffers[*pos]->buffer_ptr, 1, 1);
    trace->buffers[*pos]->buffer_cur = trace->buffers[*pos]->buffer_ptr;
}

/*
 * For internal use only.
 * Allocates an event
 */
litl_t* __litl_write_get_event(litl_trace_write_t* trace, litl_type_t type,
        litl_code_t code, int size) {

    if (trace->litl_initialized && !trace->litl_paused
            && !trace->is_buffer_full) {

        // find the thread index
        litl_med_size_t *p_index = pthread_getspecific(trace->index);
        if (!p_index) {
            __litl_write_allocate_buffer(trace);
            p_index = pthread_getspecific(trace->index);
        }
        litl_med_size_t index = *(litl_med_size_t *) p_index;

        litl_write_buffer_t *p_buffer = trace->buffers[index];

        // is there enough space in the buffer?
        if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
            // there is enough space for this event
            litl_t* cur_ptr = (litl_t*) p_buffer->buffer_cur;
            p_buffer->buffer_cur += size;

            // fill the event
            cur_ptr->time = litl_get_time();
            cur_ptr->code = code;
            cur_ptr->type = type;

            switch (type) {
            case LITL_TYPE_REGULAR:
                cur_ptr->parameters.regular.nb_params = size;
                break;
            case LITL_TYPE_RAW:
                cur_ptr->parameters.raw.size = size;
                break;
            case LITL_TYPE_PACKED:
                cur_ptr->parameters.packed.size = size;
                break;
            default:
                fprintf(stderr, "Unknown event type %d\n", type);
                abort();
            }
            return cur_ptr;
        } else if (trace->allow_buffer_flush) {

            // not enough space. flush the buffer and retry
            litl_write_flush_buffer(trace, index);
            return __litl_write_get_event(trace, type, code, size);
        } else {

            // not enough space, but flushing is disabled so just stop recording
            trace->is_buffer_full = 1;
            return NULL ;
        }
    }
    return NULL ;
}

/*
 * Records an event with offset only
 */
void litl_write_probe_offset(litl_trace_write_t* trace, litl_med_size_t index) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;
    // litl_t* cur_ptr = litl_cmpxchg((uint8_t**) &trace->buffer_cur[index],
    //    LITL_BASE_SIZE + sizeof(litl_param_t));
    litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

    cur_ptr->time = 0;
    cur_ptr->code = LITL_OFFSET_CODE;
    cur_ptr->type = LITL_TYPE_REGULAR;
    cur_ptr->parameters.offset.nb_params = 1;
    cur_ptr->parameters.offset.offset = 0;
    trace->buffers[index]->buffer_cur += LITL_BASE_SIZE + sizeof(litl_param_t);
}

/*
 * Records a regular event without any arguments
 */
void litl_write_probe_reg_0(litl_trace_write_t* trace, litl_code_t code) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 0;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE;
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_0(trace, code);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with one argument
 */
void litl_write_probe_reg_1(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 1;
        cur_ptr->parameters.regular.param[0] = param1;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_1(trace, code, param1);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with two arguments
 */
void litl_write_probe_reg_2(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 2;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 2 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_2(trace, code, param1, param2);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with three arguments
 */
void litl_write_probe_reg_3(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 3;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 3 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_3(trace, code, param1, param2, param3);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with four arguments
 */
void litl_write_probe_reg_4(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 4;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 4 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_4(trace, code, param1, param2, param3, param4);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with five arguments
 */
void litl_write_probe_reg_5(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 5;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 5 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_5(trace, code, param1, param2, param3, param4,
                param5);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with six arguments
 */
void litl_write_probe_reg_6(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 6;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 6 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_6(trace, code, param1, param2, param3, param4,
                param5, param6);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with seven arguments
 */
void litl_write_probe_reg_7(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 7;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 7 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_7(trace, code, param1, param2, param3, param4,
                param5, param6, param7);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with eight arguments
 */
void litl_write_probe_reg_8(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 8;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        cur_ptr->parameters.regular.param[7] = param8;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 8 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_8(trace, code, param1, param2, param3, param4,
                param5, param6, param7, param8);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with nine arguments
 */
void litl_write_probe_reg_9(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8, litl_param_t param9) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 9;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        cur_ptr->parameters.regular.param[7] = param8;
        cur_ptr->parameters.regular.param[8] = param9;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 9 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_9(trace, code, param1, param2, param3, param4,
                param5, param6, param7, param8, param9);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records a regular event with ten arguments
 */
void litl_write_probe_reg_10(litl_trace_write_t* trace, litl_code_t code,
        litl_param_t param1, litl_param_t param2, litl_param_t param3,
        litl_param_t param4, litl_param_t param5, litl_param_t param6,
        litl_param_t param7, litl_param_t param8, litl_param_t param9,
        litl_param_t param10) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            trace->index);
    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 10;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        cur_ptr->parameters.regular.param[7] = param8;
        cur_ptr->parameters.regular.param[8] = param9;
        cur_ptr->parameters.regular.param[9] = param10;
        trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 10 * sizeof(litl_param_t);
    } else if (trace->allow_buffer_flush) {
        litl_write_flush_buffer(trace, index);
        litl_write_probe_reg_10(trace, code, param1, param2, param3, param4,
                param5, param6, param7, param8, param9, param10);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * Records an event in a raw state, where the size is #args in the void* array.
 * That helps to discover places where the application has crashed
 */
void litl_write_probe_raw(litl_trace_write_t* trace, litl_code_t code,
        litl_size_t size, litl_data_t data[]) {
    if (!trace->litl_initialized || trace->litl_paused || trace->is_buffer_full)
        return;

    // specify explicitly the end of the string
    data[size] = '\0';
    size++;

    if (pthread_getspecific(trace->index) == NULL )
        __litl_write_allocate_buffer(trace);

    litl_med_size_t i, index;
    index = *(litl_med_size_t *) pthread_getspecific(trace->index);

    litl_t* cur_ptr = (litl_t *) trace->buffers[index]->buffer_cur;
    // needs to be done outside of the if statement 'cause of undefined size of
    //    the string that may cause segfault
    trace->buffers[index]->buffer_cur += LITL_BASE_SIZE + 7 + size;

    if (__litl_write_get_buffer_size(trace, index) < trace->buffer_size) {
        cur_ptr->time = litl_get_time();
        code = litl_set_bit(code);
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_RAW;
        cur_ptr->parameters.raw.size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                cur_ptr->parameters.raw.data[i] = data[i];
    } else if (trace->allow_buffer_flush) {
        // if there is not enough size we reset back the buffer pointer
        trace->buffers[index]->buffer_cur -= LITL_BASE_SIZE + 7 + size;

        litl_write_flush_buffer(trace, index);
        litl_write_probe_raw(trace, code, size, data);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function finalizes the trace
 */
void litl_write_finalize_trace(litl_trace_write_t* trace) {
    // write an event with the LITL_TRACE_END (= 0) code in order to indicate
    //    the end of tracing
    litl_med_size_t i;

    for (i = 0; i < trace->nb_threads; i++)
        litl_write_flush_buffer(trace, i);
    // because the LITL_TRACE_END was written to the trace buffer #0
    //    litl_write_flush_buffer(trace, 0);

    close(trace->f_handle);
    trace->f_handle = -1;

    for (i = 0; i < trace->nb_allocated_buffers; i++)
        if (trace->buffers[i]->tid != 0) {
            free(trace->buffers[i]->buffer_ptr);
        } else {
            break;
        }

    if (trace->allow_thread_safety) {
        pthread_mutex_destroy(&trace->lock_litl_flush);
    }
    pthread_mutex_destroy(&trace->lock_buffer_init);

    free(trace->filename);
    trace->filename = NULL;
    trace->litl_initialized = 0;
    trace->is_header_flushed = 0;
}
