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
#include "litl_tools.h"
#include "litl_write.h"

/*
 * Adds a header to the trace file with the information regarding:
 *   - OS
 *   - Processor type
 *   - Version of LiTL
 */
static void __litl_write_add_trace_header() {
    struct utsname uts;

    // allocate memory for the trace header
    __trace->header_ptr = (litl_buffer_t) malloc(__trace->header_size);
    if (!__trace->header_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }
    __trace->header_cur = __trace->header_ptr;

    if (uname(&uts) < 0)
        perror("Could not use uname()!");

    // get the number of symbols for litl_ver
    sprintf((char*) ((litl_header_t *) __trace->header_cur)->litl_ver, "%s",
            VERSION);

    // get the number of symbols for sysinfo
    sprintf((char*) ((litl_header_t *) __trace->header_cur)->sysinfo,
            "%s %s %s %s %s", uts.sysname, uts.nodename, uts.release,
            uts.version, uts.machine);

    // add nb_threads and buffer_size
    ((litl_header_t *) __trace->header_cur)->nb_threads = __trace->nb_threads;
    // by default a trace file per process is recorded
    ((litl_header_t *) __trace->header_cur)->is_trace_archive = 0;
    ((litl_header_t *) __trace->header_cur)->header_nb_threads =
            __trace->nb_threads;
    ((litl_header_t *) __trace->header_cur)->buffer_size = __trace->buffer_size;

    // header_size stores the position of nb_threads in the trace file
    __trace->header_size = 0;

    // size of two strings (LiTL, OS), nb_threads, and buffer_size
    __trace->header_cur += sizeof(litl_header_t);
}

/*
 * To create trace->buffer_ptr and trace->buffer_cur
 */
static void __litl_write_init_var() {
    pthread_key_create(&__trace->index, NULL );
}

/*
 * Initializes the trace buffer
 */
void litl_write_init_trace(const litl_size_t buf_size) {
    litl_med_size_t i;

    __trace = (litl_trace_write_t *) malloc(sizeof(litl_trace_write_t));

    __trace->buffer_size = buf_size;

    // set variables
    __trace->filename = NULL;
    __trace->is_buffer_full = 0;
    __trace->is_header_flushed = 0;
    litl_write_tid_recording_on();

    __trace->nb_allocated_buffers = 256;
    __trace->buffers = realloc(NULL,
            sizeof(litl_write_buffer_t*) * __trace->nb_allocated_buffers);

    for (i = 0; i < __trace->nb_allocated_buffers; i++) {
        // initialize the array already_flushed
        __trace->buffers[i] = malloc(sizeof(litl_write_buffer_t));
        __trace->buffers[i]->already_flushed = 0;

        // initialize tids by zeros; this is needed for __is_tid and __find_slot
        __trace->buffers[i]->tid = 0;
    }
    __trace->nb_threads = 0;

    // initialize the timing mechanism
    litl_time_initialize();

    // a jump function is needed 'cause it is not possible to pass args to
    //   the calling function through pthread_once
    void __init() {
        __litl_write_init_var();
    }
    __trace->index_once = PTHREAD_ONCE_INIT;
    pthread_once(&__trace->index_once, __init);

    // set trace.allow_buffer_flush using the environment variable.
    //   By default the buffer flushing is enabled
    char* str = getenv("LITL_BUFFER_FLUSH");
    if (str && (strcmp(str, "off") == 0))
        litl_write_buffer_flush_off();
    else
        litl_write_buffer_flush_on();

    // set trace.allow_thread_safety using the environment variable.
    //   By default thread safety is enabled
    str = getenv("LITL_THREAD_SAFETY");
    if (str && (strcmp(str, "off") == 0))
        litl_write_thread_safety_off();
    else
        litl_write_thread_safety_on();

    if (__trace->allow_thread_safety)
        pthread_mutex_init(&__trace->lock_litl_flush, NULL );
    pthread_mutex_init(&__trace->lock_buffer_init, NULL );

    // TODO: touch each block in buffer_ptr in order to load it

    __trace->litl_paused = 0;
    __trace->litl_initialized = 1;
}

/*
 * Computes the size of data in the trace header
 */
static litl_size_t __litl_write_get_header_size() {
    return (__trace->header_cur - __trace->header_ptr);
}

/*
 * Computes the size of data in buffer
 */
static litl_size_t __litl_write_get_buffer_size(litl_med_size_t pos) {
    return (__trace->buffers[pos]->buffer_cur
            - __trace->buffers[pos]->buffer_ptr);
}

/*
 * Activates buffer flush
 */
void litl_write_buffer_flush_on() {
    __trace->allow_buffer_flush = 1;
}

/*
 * Deactivates buffer flush. By default, it is activated
 */
void litl_write_buffer_flush_off() {
    __trace->allow_buffer_flush = 0;
}

/*
 * Activate thread safety. By default it is deactivated
 */
void litl_write_thread_safety_on() {
    __trace->allow_thread_safety = 1;
}

/*
 * Deactivates thread safety
 */
void litl_write_thread_safety_off() {
    __trace->allow_thread_safety = 0;
}

/*
 * Activates recording tid. By default it is deactivated
 */
void litl_write_tid_recording_on() {
    __trace->record_tid_activated = 1;
}

/*
 * Deactivates recording tid
 */
void litl_write_tid_recording_off() {
    __trace->record_tid_activated = 0;
}

/*
 * Pauses the event recording
 */
void litl_write_pause_recording() {
    if (__trace)
        __trace->litl_paused = 1;
}

/*
 * Resumes the event recording
 */
void litl_write_resume_recording() {
    if (__trace)
        __trace->litl_paused = 0;
}

/*
 * Sets a new name for the trace file
 */
void litl_write_set_filename(char* filename) {
    if (__trace->filename) {
        if (__trace->is_header_flushed)
            fprintf(stderr,
                    "Warning: changing the trace file name to %s after some events have been saved in file %s\n",
                    filename, __trace->filename);
        free(__trace->filename);
    }

    // check whether the file name was set. If no, set it by default trace name.
    if (filename == NULL )
        sprintf(filename, "/tmp/%s_%s", getenv("USER"), "litl_log_1");

    if (asprintf(&__trace->filename, "%s", filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Records an event with offset only
 */
static void __litl_write_probe_offset(litl_med_size_t index) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;
    // litl_t* cur_ptr = litl_cmpxchg((uint8_t**) &trace->buffer_cur[index],
    //    LITL_BASE_SIZE + sizeof(litl_param_t));
    litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

    cur_ptr->time = 0;
    cur_ptr->code = LITL_OFFSET_CODE;
    cur_ptr->type = LITL_TYPE_REGULAR;
    cur_ptr->parameters.offset.nb_params = 1;
    cur_ptr->parameters.offset.offset = 0;
    __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
            + sizeof(litl_param_t);
}

/*
 * Writes the recorded events from the buffer to the trace file
 */
static void __litl_write_flush_buffer(litl_med_size_t index) {
    int res __attribute__ ((__unused__));

    if (!__trace->litl_initialized)
        return;

    if (__trace->allow_thread_safety)
        pthread_mutex_lock(&__trace->lock_litl_flush);

    if (!__trace->is_header_flushed) {
        // check whether the trace file can be opened
        if ((__trace->f_handle = open(__trace->filename, O_WRONLY | O_CREAT,
                0644)) < 0) {
            fprintf(stderr, "Cannot open %s\n", __trace->filename);
            exit(EXIT_FAILURE);
        }

        // add a header to the trace file
        __trace->header_size = sizeof(litl_header_t)
                + (__trace->nb_threads + 1) * sizeof(litl_header_tids_t);
        __litl_write_add_trace_header();

        // add information about each working thread: (tid, offset)
        litl_med_size_t i;
        for (i = 0; i < __trace->nb_threads; i++) {
            ((litl_header_tids_t *) __trace->header_cur)->tid =
                    __trace->buffers[i]->tid;
            ((litl_header_tids_t *) __trace->header_cur)->offset = 0;

            __trace->header_cur += sizeof(litl_header_tids_t);

            // save the position of offset inside the trace file
            __trace->buffers[i]->offset = __litl_write_get_header_size()
                    - sizeof(litl_offset_t);
            __trace->buffers[i]->already_flushed = 1;
        }

        // offset indicates the position of offset to the next slot of
        //   pairs (tid, offset) within the trace file
        __trace->header_offset = __litl_write_get_header_size();

        // specify the last slot of pairs (offset == 0)
        ((litl_header_tids_t *) __trace->header_cur)->tid = 0;
        ((litl_header_tids_t *) __trace->header_cur)->offset = 0;
        __trace->header_cur += sizeof(litl_header_tids_t);

        // write the trace header to the trace file
        if (write(__trace->f_handle, __trace->header_ptr,
                __litl_write_get_header_size()) == -1) {
            perror(
                    "Flushing the buffer. Could not write measured data to the trace file!");
            exit(EXIT_FAILURE);
        }

        // set the general_offset
        __trace->general_offset = __litl_write_get_header_size();

        __trace->header_nb_threads = __trace->nb_threads;
        __trace->threads_offset = 0;
        __trace->nb_slots = 0;

        __trace->is_header_flushed = 1;
    }

    // handle the situation when some threads start after the header was flushed
    if (!__trace->buffers[index]->already_flushed) {

        // when more buffers to store threads information is required
        if (__trace->nb_threads
                > (__trace->header_nb_threads + NBTHREADS * __trace->nb_slots)) {

            // updated the offset from the previous slot
            lseek(__trace->f_handle,
                    __trace->header_offset + sizeof(litl_tid_t), SEEK_SET);
            res = write(__trace->f_handle, &__trace->general_offset,
                    sizeof(litl_offset_t));

            // reserve a new slot for pairs (tid, offset)
            __trace->header_offset = __trace->general_offset;
            __trace->threads_offset = __trace->header_offset;
            __trace->general_offset += (NBTHREADS + 1)
                    * sizeof(litl_header_tids_t);

            __trace->nb_slots++;
        }

        // add a new pair (tid, offset)
        lseek(__trace->f_handle, __trace->header_offset, SEEK_SET);
        res = write(__trace->f_handle, &__trace->buffers[index]->tid,
                sizeof(litl_tid_t));
        res = write(__trace->f_handle, &__trace->general_offset,
                sizeof(litl_offset_t));

        // add an indicator to specify the last slot of pairs (offset == 0)
        // TODO: how to optimize this and write only once at the end of the slot
        res = write(__trace->f_handle,
                &__trace->buffers[index]->already_flushed, sizeof(litl_tid_t));
        res = write(__trace->f_handle,
                &__trace->buffers[index]->already_flushed,
                sizeof(litl_offset_t));

        __trace->header_offset += sizeof(litl_header_tids_t);
        __trace->buffers[index]->already_flushed = 1;

        // updated the number of threads
        // TODO: perform update only once 'cause there is duplication
        //        printf("trace->nb_threads = %d\n", trace->nb_threads);
        lseek(__trace->f_handle, __trace->header_size, SEEK_SET);
        res = write(__trace->f_handle, &__trace->nb_threads,
                sizeof(litl_med_size_t));
        lseek(__trace->f_handle, __trace->general_offset, SEEK_SET);
    } else {
        // update the previous offset of the current thread,
        //   updating the location in the file
        lseek(__trace->f_handle, __trace->buffers[index]->offset, SEEK_SET);
        res = write(__trace->f_handle, &__trace->general_offset,
                sizeof(litl_offset_t));
        lseek(__trace->f_handle, __trace->general_offset, SEEK_SET);
    }

    // add an event with offset
    __litl_write_probe_offset(index);
    if (write(__trace->f_handle, __trace->buffers[index]->buffer_ptr,
            __litl_write_get_buffer_size(index)) == -1) {
        perror(
                "Flushing the buffer. Could not write measured data to the trace file!");
        abort();
    }

    // update the general_offset
    __trace->general_offset += __litl_write_get_buffer_size(index);
    // update the current offset of the thread
    __trace->buffers[index]->offset = __trace->general_offset
            - sizeof(litl_offset_t);

    if (__trace->allow_thread_safety)
        pthread_mutex_unlock(&__trace->lock_litl_flush);

    __trace->buffers[index]->buffer_cur = __trace->buffers[index]->buffer_ptr;
}

/*
 * Checks whether the trace buffer was allocated. If no, then allocate
 *    the buffer and, for otherwise too, returns the position of
 *    the thread buffer in the array buffer_ptr/buffer_cur.
 */
static void __litl_write_allocate_buffer() {
    litl_med_size_t* pos;

    // thread safe region
    pthread_mutex_lock(&__trace->lock_buffer_init);

    pos = malloc(sizeof(litl_med_size_t));
    *pos = __trace->nb_threads;
    pthread_setspecific(__trace->index, pos);
    __trace->nb_threads++;

    if (*pos >= __trace->nb_allocated_buffers) {
        // We need to allocate a bigger array of buffers
        void* ptr = realloc(__trace->buffers,
                __trace->nb_allocated_buffers * 2
                        * sizeof(litl_write_buffer_t*));
        if (!ptr) {
            fprintf(stderr, "LiTL failed to allocate memory. Aborting.\n");
            abort();
        }

        __trace->buffers = ptr;
        unsigned i;
        for (i = __trace->nb_allocated_buffers;
                i < 2 * __trace->nb_allocated_buffers; i++) {
            __trace->buffers[i] = malloc(sizeof(litl_write_buffer_t));
            __trace->buffers[i]->already_flushed = 0;
        }
        __trace->nb_allocated_buffers *= 2;
    }

    __trace->buffers[*pos]->tid = CUR_TID;
    __trace->buffers[*pos]->already_flushed = 0;

    pthread_mutex_unlock(&__trace->lock_buffer_init);

    __trace->buffers[*pos]->buffer_ptr = malloc(
            __trace->buffer_size + __litl_get_reg_event_size(LITL_MAX_PARAMS)
                    + __litl_get_reg_event_size(1));
    if (!__trace->buffers[*pos]->buffer_ptr) {
        perror("Could not allocate memory for the buffer!");
        exit(EXIT_FAILURE);
    }

    // touch the memory so that it is allocated for real (otherwise, this may
    //    cause performance issues on NUMA machines)
    memset(__trace->buffers[*pos]->buffer_ptr, 1, 1);
    __trace->buffers[*pos]->buffer_cur = __trace->buffers[*pos]->buffer_ptr;
}

/*
 * For internal use only.
 * Allocates an event
 */
litl_t* __litl_write_get_event(litl_type_t type, litl_code_t code, int size) {

    if (__trace->litl_initialized && !__trace->litl_paused
            && !__trace->is_buffer_full) {

        // find the thread index
        litl_med_size_t *p_index = pthread_getspecific(__trace->index);
        if (!p_index) {
            __litl_write_allocate_buffer();
            p_index = pthread_getspecific(__trace->index);
        }
        litl_med_size_t index = *(litl_med_size_t *) p_index;

        litl_write_buffer_t *p_buffer = __trace->buffers[index];

        // is there enough space in the buffer?
        if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
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
        } else if (__trace->allow_buffer_flush) {

            // not enough space. flush the buffer and retry
            __litl_write_flush_buffer(index);
            return __litl_write_get_event(type, code, size);
        } else {

            // not enough space, but flushing is disabled so just stop recording
            __trace->is_buffer_full = 1;
            return NULL ;
        }
    }
    return NULL ;
}

/*
 * Records a regular event without any arguments
 */
void litl_write_probe_reg_0(litl_code_t code) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 0;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE;
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_0(code);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with one argument
 */
void litl_write_probe_reg_1(litl_code_t code, litl_param_t param1) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 1;
        cur_ptr->parameters.regular.param[0] = param1;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_1(code, param1);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with two arguments
 */
void litl_write_probe_reg_2(litl_code_t code, litl_param_t param1,
        litl_param_t param2) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 2;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 2 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_2(code, param1, param2);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with three arguments
 */
void litl_write_probe_reg_3(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 3;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 3 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_3(code, param1, param2, param3);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with four arguments
 */
void litl_write_probe_reg_4(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 4;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 4 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_4(code, param1, param2, param3, param4);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with five arguments
 */
void litl_write_probe_reg_5(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 5;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 5 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_5(code, param1, param2, param3, param4, param5);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with six arguments
 */
void litl_write_probe_reg_6(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

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
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 6 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_6(code, param1, param2, param3, param4, param5,
                param6);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with seven arguments
 */
void litl_write_probe_reg_7(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

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
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 7 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_7(code, param1, param2, param3, param4, param5,
                param6, param7);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with eight arguments
 */
void litl_write_probe_reg_8(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

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
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 8 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_8(code, param1, param2, param3, param4, param5,
                param6, param7, param8);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with nine arguments
 */
void litl_write_probe_reg_9(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8, litl_param_t param9) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

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
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 9 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_9(code, param1, param2, param3, param4, param5,
                param6, param7, param8, param9);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records a regular event with ten arguments
 */
void litl_write_probe_reg_10(litl_code_t code, litl_param_t param1,
        litl_param_t param2, litl_param_t param3, litl_param_t param4,
        litl_param_t param5, litl_param_t param6, litl_param_t param7,
        litl_param_t param8, litl_param_t param9, litl_param_t param10) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t index = *(litl_med_size_t *) pthread_getspecific(
            __trace->index);
    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;

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
        __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE
                + 10 * sizeof(litl_param_t);
    } else if (__trace->allow_buffer_flush) {
        __litl_write_flush_buffer(index);
        litl_write_probe_reg_10(code, param1, param2, param3, param4, param5,
                param6, param7, param8, param9, param10);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * Records an event in a raw state, where the size is #args in the void* array.
 * That helps to discover places where the application has crashed
 */
void litl_write_probe_raw(litl_code_t code, litl_size_t size,
        litl_data_t data[]) {
    if (!__trace->litl_initialized || __trace->litl_paused
            || __trace->is_buffer_full)
        return;

    // specify explicitly the end of the string
    data[size] = '\0';
    size++;

    if (pthread_getspecific(__trace->index) == NULL )
        __litl_write_allocate_buffer();

    litl_med_size_t i, index;
    index = *(litl_med_size_t *) pthread_getspecific(__trace->index);

    litl_t* cur_ptr = (litl_t *) __trace->buffers[index]->buffer_cur;
    // needs to be done outside of the if statement 'cause of undefined size of
    //    the string that may cause segfault
    __trace->buffers[index]->buffer_cur += LITL_BASE_SIZE + 7 + size;

    if (__litl_write_get_buffer_size(index) < __trace->buffer_size) {
        cur_ptr->time = litl_get_time();
        cur_ptr->code = code;
        cur_ptr->type = LITL_TYPE_RAW;
        cur_ptr->parameters.raw.size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                cur_ptr->parameters.raw.data[i] = data[i];
    } else if (__trace->allow_buffer_flush) {
        // if there is not enough size we reset back the buffer pointer
        __trace->buffers[index]->buffer_cur -= LITL_BASE_SIZE + 7 + size;

        __litl_write_flush_buffer(index);
        litl_write_probe_raw(code, size, data);
    } else
        // this applies only when the flushing is off
        __trace->is_buffer_full = 1;
}

/*
 * This function finalizes the trace
 */
void litl_write_finalize_trace() {
    // write an event with the LITL_TRACE_END (= 0) code in order to indicate
    //    the end of tracing
    litl_med_size_t i;

    for (i = 0; i < __trace->nb_threads; i++)
        __litl_write_flush_buffer(i);
    // because the LITL_TRACE_END was written to the trace buffer #0
    //    __litl_write_flush_buffer(trace, 0);

    close(__trace->f_handle);
    __trace->f_handle = -1;

    for (i = 0; i < __trace->nb_allocated_buffers; i++)
        if (__trace->buffers[i]->tid != 0) {
            free(__trace->buffers[i]->buffer_ptr);
        } else {
            break;
        }

    if (__trace->allow_thread_safety) {
        pthread_mutex_destroy(&__trace->lock_litl_flush);
    }
    pthread_mutex_destroy(&__trace->lock_buffer_init);

    free(__trace->filename);
    __trace->filename = NULL;
    __trace->litl_initialized = 0;
    __trace->is_header_flushed = 0;
}
