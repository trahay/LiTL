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

#include "timer.h"
#include "evnt_macro.h"
#include "evnt_write.h"

/*
 * This function adds a header to the trace file the information regarding:
 *   - OS
 *   - Processor type
 *   - Version of libevnt
 */
static void __add_trace_header(evnt_trace_write_t* trace) {
    struct utsname uts;

    // allocate memory for the trace header
    trace->header_ptr = (evnt_buffer_t) malloc(trace->header_size);
    if (!trace->header_ptr) {
        perror("Could not allocate memory for the trace header!");
        exit(EXIT_FAILURE);
    }
    trace->header_cur = trace->header_ptr;

    if (uname(&uts) < 0)
        perror("Could not use uname()!");

    // get the number of symbols for libevnt_ver
    sprintf((char*) ((evnt_header_t *) trace->header_cur)->libevnt_ver, "%s", VERSION);

    // get the number of symbols for sysinfo
    sprintf((char*) ((evnt_header_t *) trace->header_cur)->sysinfo, "%s %s %s %s %s", uts.sysname, uts.nodename,
            uts.release, uts.version, uts.machine);

    // add nb_threads and buffer_size
    ((evnt_header_t *) trace->header_cur)->nb_threads = 0;
    ((evnt_header_t *) trace->header_cur)->buffer_size = trace->buffer_size;

    // size of two strings (libevnt, OS), nb_threads, and buffer_size
    trace->header_cur += sizeof(evnt_header_t);
}

/*
 * To create trace->buffer_ptr and trace->buffer_cur
 */
static void __init_var(evnt_trace_write_t* trace) {
    pthread_key_create(&trace->index, NULL );
}

/*
 * This function initializes the trace
 */
evnt_trace_write_t evnt_init_trace(const uint32_t buf_size) {
    evnt_size_t i;
    evnt_trace_write_t trace;

    trace.buffer_size = buf_size;

    // set variables
    trace.header_size = 1536; // 1.5Kb
    trace.evnt_filename = NULL;
    trace.is_header_flushed = 0;
    trace.is_buffer_full = 0;
    evnt_tid_recording_on(&trace);

    // initialize the array already_flushed
    for (i = 0; i < NBBUFFER; i++)
        trace.already_flushed[i] = 0;

    // initialize tids by zeros; this is needed for __is_tid and __find_slot
    for (i = 0; i < NBBUFFER; i++)
        trace.tids[i] = 0;
    trace.nb_threads = 0;

    // a jump function is needed 'cause it is not possible to pass args to the calling function through pthread_once
    void __init() {
        __init_var(&trace);
    }
    trace.index_once = PTHREAD_ONCE_INIT;
    pthread_once(&trace.index_once, __init);

    // set trace.allow_buffer_flush using the environment variable. By default the flushing is enabled
    char* str = getenv("EVNT_BUFFER_FLUSH");
    if (str && (strcmp(str, "off") == 0))
        evnt_buffer_flush_off(&trace);
    else
        evnt_buffer_flush_on(&trace);

    // set trace.allow_thread_safety using the environment variable. By default thread safety is enabled
    str = getenv("EVNT_THREAD_SAFETY");
    if (str && (strcmp(str, "off") == 0))
        evnt_thread_safety_off(&trace);
    else
        evnt_thread_safety_on(&trace);

    if (trace.allow_thread_safety)
        pthread_mutex_init(&trace.lock_evnt_flush, NULL );
    pthread_mutex_init(&trace.lock_buffer_init, NULL );

    // TODO: touch each block in buffer_ptr in order to load it
    trace.evnt_paused = 0;
    trace.evnt_initialized = 1;

    // add a header to the trace file
    __add_trace_header(&trace);

    return trace;
}

/*
 * This function computes the size of data in the trace header
 */
static uint32_t __get_header_size(evnt_trace_write_t* trace) {
    return (trace->header_cur - trace->header_ptr);
}

/*
 * This function computes the size of data in buffer
 */
static uint32_t __get_buffer_size(evnt_trace_write_t* trace, evnt_size_t pos) {
    return (trace->buffer_cur[pos] - trace->buffer_ptr[pos]);
}

/*
 * Activate buffer flush
 */
void evnt_buffer_flush_on(evnt_trace_write_t* trace) {
    trace->allow_buffer_flush = 1;
}

/*
 * Deactivate buffer flush. It is activated by default
 */
void evnt_buffer_flush_off(evnt_trace_write_t* trace) {
    trace->allow_buffer_flush = 0;
}

/*
 * Activate thread safety. It is not activated by default
 */
void evnt_thread_safety_on(evnt_trace_write_t* trace) {
    trace->allow_thread_safety = 1;
}

/*
 * Deactivate thread safety
 */
void evnt_thread_safety_off(evnt_trace_write_t* trace) {
    trace->allow_thread_safety = 0;
}

/*
 * Activate recording tid. It is not activated by default
 */
void evnt_tid_recording_on(evnt_trace_write_t* trace) {
    trace->record_tid_activated = 1;
}

/*
 * Deactivate recording tid
 */
void evnt_tid_recording_off(evnt_trace_write_t* trace) {
    trace->record_tid_activated = 0;
}

void evnt_pause_recording(evnt_trace_write_t* trace) {
    trace->evnt_paused = 1;
}

void evnt_resume_recording(evnt_trace_write_t* trace) {
    trace->evnt_paused = 0;
}

/*
 * Set a new name for the trace file
 */
void evnt_set_filename(evnt_trace_write_t* trace, char* filename) {
    if (trace->evnt_filename) {
        if (trace->is_header_flushed)
            fprintf(stderr,
                    "Warning: changing the trace file name to %s after some events have been saved in file %s\n",
                    filename, trace->evnt_filename);
        free(trace->evnt_filename);
    }

    // check whether the file name was set. If no, set it by default trace name.
    if (filename == NULL )
        sprintf(filename, "/tmp/%s_%s", getenv("USER"), "eztrace_log_rank_1");

    if (asprintf(&trace->evnt_filename, filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void evnt_flush_buffer(evnt_trace_write_t* trace, evnt_size_t index) {
    if (!trace->evnt_initialized)
        return;

    if (trace->allow_thread_safety)
        pthread_mutex_lock(&trace->lock_evnt_flush);

    if (!trace->is_header_flushed) {
        // check whether the trace file can be opened
        if (!(trace->ftrace = fopen(trace->evnt_filename, "w+"))) {
            perror("Could not open the trace file for writing!");
            exit(EXIT_FAILURE);
        }

        /*
         TODO: handling more than 64 threads
         if (__get_header_size(trace) < trace->header_size)
         // relocate memory
         */

        // update nb_threads
        *(evnt_size_t *) (trace->header_cur - 2 * sizeof(evnt_size_t)) = trace->nb_threads;
        // header_size stores the position of nb_threads in the trace file
        trace->header_size = __get_header_size(trace) - 2 * sizeof(evnt_size_t);

        // add information about each working thread: (tid, offset)
        // put first the information regarding the current thread
        evnt_size_t i;
        for (i = 0; i < trace->nb_threads; i++) {
            ((evnt_header_tids_t *) trace->header_cur)->tid = trace->tids[i];
            ((evnt_header_tids_t *) trace->header_cur)->offset = 0;

            trace->header_cur += sizeof(evnt_tid_t) + sizeof(evnt_offset_t);
            // save the position of offset inside the trace file
            trace->offsets[i] = __get_header_size(trace) - sizeof(evnt_offset_t);
            trace->already_flushed[i] = 1;
        }

        // increase the size of header to be able to hold exactly 64 pairs of tid and offset.
        // TODO: if the previous todo changes, this also needs to be modified
        if (trace->nb_threads < 64) {
            // offset from the top of the trace to the next free position for a pair (tid, offset)
            trace->header_offset = __get_header_size(trace);
            // the header should hold information about all 64 threads
            trace->header_cur += (64 - trace->nb_threads) * (sizeof(evnt_tid_t) + sizeof(evnt_offset_t));
        }

        // write the trace header to the trace file
        if (fwrite(trace->header_ptr, __get_header_size(trace), 1, trace->ftrace) != 1) {
            perror("Flushing the buffer. Could not write measured data to the trace file!");
            exit(EXIT_FAILURE);
        }

        // set the general_offset
        trace->general_offset = __get_header_size(trace);

        trace->is_header_flushed = 1;
    }

    // handle the situation when some threads start after the header was flushed
    if (!trace->already_flushed[index]) {
        // add the pair tid and add & update offset at once
        fseek(trace->ftrace, trace->header_offset, SEEK_SET);
        fwrite(&trace->tids[index], sizeof(evnt_tid_t), 1, trace->ftrace);
        fwrite(&trace->general_offset, sizeof(evnt_offset_t), 1, trace->ftrace);
        fseek(trace->ftrace, trace->general_offset, SEEK_SET);

        trace->header_offset += sizeof(evnt_tid_t) + sizeof(evnt_offset_t);
        trace->already_flushed[index] = 1;

        // updated the number of threads
        fseek(trace->ftrace, trace->header_size, SEEK_SET);
        fwrite(&trace->nb_threads, sizeof(evnt_size_t), 1, trace->ftrace);
        fseek(trace->ftrace, trace->general_offset, SEEK_SET);
    } else {
        // update the previous offset of the current thread, updating the location in the file
        fseek(trace->ftrace, trace->offsets[index], SEEK_SET);
        fwrite(&trace->general_offset, sizeof(evnt_offset_t), 1, trace->ftrace);
        fseek(trace->ftrace, trace->general_offset, SEEK_SET);
    }

    // add an event with offset
    evnt_probe_offset(trace, index);
    if (fwrite(trace->buffer_ptr[index], __get_buffer_size(trace, index), 1, trace->ftrace) != 1) {
        perror("Flushing the buffer. Could not write measured data to the trace file!");
        abort();
        exit(EXIT_FAILURE);
    }

    // update the general_offset
    trace->general_offset += __get_buffer_size(trace, index);
    // update the current offset of the thread
    trace->offsets[index] = trace->general_offset - sizeof(evnt_offset_t);

    if (trace->allow_thread_safety)
        pthread_mutex_unlock(&trace->lock_evnt_flush);

    trace->buffer_cur[index] = trace->buffer_ptr[index];
}

/*
 * Checks whether the trace buffer was allocated. If no, then allocate the buffer and, for otherwise too, returns
 *      the position of the thread buffer in the array buffer_ptr/buffer_cur.
 */
static void __allocate_buffer(evnt_trace_write_t* trace) {
    evnt_size_t* pos;

    // thread safe region
    pthread_mutex_lock(&trace->lock_buffer_init);

    pos = malloc(sizeof(evnt_size_t));
    *pos = trace->nb_threads;
    pthread_setspecific(trace->index, pos);
    trace->nb_threads++;

    trace->tids[*pos] = CUR_TID;

    pthread_mutex_unlock(&trace->lock_buffer_init);

    trace->buffer_ptr[*pos] = malloc(trace->buffer_size + get_event_size(EVNT_MAX_PARAMS) + get_event_size(1));
    if (!trace->buffer_ptr[*pos]) {
        perror("Could not allocate memory for the buffer!");
        exit(EXIT_FAILURE);
    }
    trace->buffer_cur[*pos] = trace->buffer_ptr[*pos];
}

evnt_t* get_event(evnt_trace_write_t* trace, evnt_type_t type, evnt_code_t code, int size) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return NULL ;

    evnt_size_t *p_index = pthread_getspecific(trace->index);
    if(!p_index) {
        __allocate_buffer(trace);
	p_index = pthread_getspecific(trace->index);
    }

    evnt_size_t index = *(evnt_size_t *) p_index;


    retry: if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t*) trace->buffer_cur[index];
        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = type;
        switch (type) {
        case EVNT_TYPE_REGULAR:
            cur_ptr->parameters.regular.nb_params = size;
            break;
        case EVNT_TYPE_RAW:
            cur_ptr->parameters.raw.size = size;
            break;
        case EVNT_TYPE_PACKED:
            cur_ptr->parameters.packed.size = size;
            break;
        default:
            fprintf(stderr, "Unknown event type %d\n", type);
            abort();
        }
        trace->buffer_cur[index] += size;
        return cur_ptr;
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        goto retry;
    } else {
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
        return NULL ;
    }
}

/*
 * This function records an event with offset only
 */
void evnt_probe_offset(evnt_trace_write_t* trace, int16_t index) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;
//    evnt_t* cur_ptr = evnt_cmpxchg((uint8_t**) &trace->buffer_cur[index], EVNT_BASE_SIZE + sizeof(evnt_param_t));
    evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

    cur_ptr->time = 0;
    cur_ptr->code = EVNT_OFFSET_CODE;
    cur_ptr->type = EVNT_TYPE_REGULAR;
    cur_ptr->parameters.offset.nb_params = 1;
    cur_ptr->parameters.offset.offset = 0;
    trace->buffer_cur[index] += EVNT_BASE_SIZE + sizeof(evnt_param_t);
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(evnt_trace_write_t* trace, evnt_code_t code) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 0;
        trace->buffer_cur[index] += EVNT_BASE_SIZE;
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe0(trace, code);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 1;
        cur_ptr->parameters.regular.param[0] = param1;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe1(trace, code, param1);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 2;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 2 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe2(trace, code, param1, param2);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 3;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 3 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe3(trace, code, param1, param2, param3);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 4;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 4 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe4(trace, code, param1, param2, param3, param4);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 5;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 5 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe5(trace, code, param1, param2, param3, param4, param5);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5, evnt_param_t param6) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 6;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 6 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe6(trace, code, param1, param2, param3, param4, param5, param6);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 7;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 7 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe7(trace, code, param1, param2, param3, param4, param5, param6, param7);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7,
        evnt_param_t param8) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
        cur_ptr->parameters.regular.nb_params = 8;
        cur_ptr->parameters.regular.param[0] = param1;
        cur_ptr->parameters.regular.param[1] = param2;
        cur_ptr->parameters.regular.param[2] = param3;
        cur_ptr->parameters.regular.param[3] = param4;
        cur_ptr->parameters.regular.param[4] = param5;
        cur_ptr->parameters.regular.param[5] = param6;
        cur_ptr->parameters.regular.param[6] = param7;
        cur_ptr->parameters.regular.param[7] = param8;
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 8 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe8(trace, code, param1, param2, param3, param4, param5, param6, param7, param8);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7,
        evnt_param_t param8, evnt_param_t param9) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
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
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 9 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe9(trace, code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with ten arguments
 */
void evnt_probe10(evnt_trace_write_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2,
        evnt_param_t param3, evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7,
        evnt_param_t param8, evnt_param_t param9, evnt_param_t param10) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t index = *(evnt_size_t *) pthread_getspecific(trace->index);
    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];

        cur_ptr->time = evnt_get_time();
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_REGULAR;
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
        trace->buffer_cur[index] += EVNT_BASE_SIZE + 10 * sizeof(evnt_param_t);
    } else if (trace->allow_buffer_flush) {
        evnt_flush_buffer(trace, index);
        evnt_probe10(trace, code, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event in a raw state, where the size is the number of chars in the array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(evnt_trace_write_t* trace, evnt_code_t code, evnt_size_t size, evnt_data_t data[]) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    if (pthread_getspecific(trace->index) == NULL )
        __allocate_buffer(trace);

    evnt_size_t i, index;
    index = *(evnt_size_t *) pthread_getspecific(trace->index);

    evnt_t* cur_ptr = (evnt_t *) trace->buffer_cur[index];
    // needs to be done outside of the if statement 'cause of undefined size of the string which may cause segfault
    trace->buffer_cur[index] += EVNT_BASE_SIZE + 7 + size;

    if (__get_buffer_size(trace, index) < trace->buffer_size) {
        cur_ptr->time = evnt_get_time();
        code = set_bit(code);
        cur_ptr->code = code;
        cur_ptr->type = EVNT_TYPE_RAW;
        cur_ptr->parameters.raw.size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                cur_ptr->parameters.raw.data[i] = data[i];
    } else if (trace->allow_buffer_flush) {
        // if there is not enough size we reset back the buffer pointer
        trace->buffer_cur[index] -= EVNT_BASE_SIZE + 7 + size;

        evnt_flush_buffer(trace, index);
        evnt_raw_probe(trace, code, size, data);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function finalizes the trace
 */
void evnt_fin_trace(evnt_trace_write_t* trace) {
    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    evnt_size_t i;

    for (i = 0; i < trace->nb_threads; i++)
        evnt_flush_buffer(trace, i);
    // because the EVNT_TRACE_END was written to the trace buffer #0
    //    evnt_flush_buffer(trace, 0);

    fclose(trace->ftrace);

    for (i = 0; i < NBBUFFER; i++)
        if (trace->tids[i] != 0)
            free(trace->buffer_ptr[i]);
        else
            break;

    if (trace->allow_thread_safety)
        pthread_mutex_destroy(&trace->lock_evnt_flush);
    pthread_mutex_destroy(&trace->lock_buffer_init);

    trace->ftrace = NULL;
    free(trace->evnt_filename);
    trace->evnt_filename = NULL;
    trace->evnt_initialized = 0;
    trace->is_header_flushed = 0;
}
