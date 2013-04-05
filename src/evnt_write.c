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
 * Implementation of thread safety through atomic compare and swap operation
 */
#define evnt_cmpxchg(ptr, obj1, obj2) \
        __sync_bool_compare_and_swap((ptr), (obj1), (obj2))

/*
 * This function adds a header to the trace file the information regarding:
 *   - OS
 *   - Processor type
 *   - Version of libevnt
 */
static void add_trace_header(evnt_trace_t* trace) {
    int n, size;
    struct utsname uts;

    if (uname(&uts) < 0)
        perror("Could not use uname()!");

    // get the number of symbols for libevnt_ver
    n = sprintf(((evnt_info_t *) trace->buffer_cur)->libevnt_ver, "%s", VERSION);
    // +1 corresponds to the '\0' symbol
    size = n + 1;

    // get the number of symbols for sysinfo
    n = sprintf(((evnt_info_t *) trace->buffer_cur)->sysinfo, "%s %s %s %s %s", uts.sysname, uts.nodename, uts.release,
            uts.version, uts.machine);
    // +1 corresponds to the '\0' symbol
    size += n + 1;

    trace->buffer_cur += (evnt_param_t) ceil((double) size / sizeof(evnt_param_t));
}

/*
 * This function initializes the trace
 */
evnt_trace_t evnt_init_trace(const uint32_t buf_size) {
    void *vp;
    evnt_trace_t trace;

    trace.buffer_size = buf_size;

    // the size of the buffer is slightly bigger than it was required, because one additional event is added after the tracing
    vp = malloc(buf_size + get_event_size(EVNT_MAX_PARAMS));
    if (!vp) {
        perror("Could not allocate memory for the buffer!");
        exit(EXIT_FAILURE);
    }

    // set variables
    trace.buffer_ptr = vp;
    trace.buffer_cur = trace.buffer_ptr;
    trace.evnt_filename = NULL;
    trace.already_flushed = 0;
    trace.is_buffer_full = 0;
    evnt_tid_recording_on(&trace);

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

    // add a header to the trace file
    add_trace_header(&trace);

    // TODO: touch each block in buffer_ptr in order to load it

    trace.evnt_paused = 0;
    trace.evnt_initialized = 1;

    return trace;
}

/*
 * This function computes the size of data in buffer
 */
static uint32_t __get_buffer_size(evnt_trace_t* trace) {
    return sizeof(evnt_buffer_t) * ((evnt_buffer_t) trace->buffer_cur - (evnt_buffer_t) trace->buffer_ptr);
}

/*
 * Activate buffer flush
 */
void evnt_buffer_flush_on(evnt_trace_t* trace) {
    trace->allow_buffer_flush = 1;
}

/*
 * Deactivate buffer flush. It is activated by default
 */
void evnt_buffer_flush_off(evnt_trace_t* trace) {
    trace->allow_buffer_flush = 0;
}

/*
 * Activate thread safety. It is not activated by default
 */
void evnt_thread_safety_on(evnt_trace_t* trace) {
    trace->allow_thread_safety = 1;
}

/*
 * Deactivate thread safety
 */
void evnt_thread_safety_off(evnt_trace_t* trace) {
    trace->allow_thread_safety = 0;
}

/*
 * Activate recording tid. It is not activated by default
 */
void evnt_tid_recording_on(evnt_trace_t* trace) {
    trace->record_tid_activated = 1;
}

/*
 * Deactivate recording tid
 */
void evnt_tid_recording_off(evnt_trace_t* trace) {
    trace->record_tid_activated = 0;
}

void evnt_pause_recording(evnt_trace_t* trace) {
    trace->evnt_paused = 1;
}

void evnt_resume_recording(evnt_trace_t* trace) {
    trace->evnt_paused = 0;
}

/*
 * Set a new name for the trace file
 */
void evnt_set_filename(evnt_trace_t* trace, char* filename) {
    if (trace->evnt_filename) {
        if (trace->already_flushed)
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
void evnt_flush_buffer(evnt_trace_t* trace) {
    if (!trace->evnt_initialized)
        return;

    if (trace->allow_thread_safety)
        pthread_mutex_lock(&trace->lock_evnt_flush);

    if (!trace->already_flushed)
        // check whether the trace file can be opened
        if (!(trace->ftrace = fopen(trace->evnt_filename, "w+"))) {
            perror("Could not open the trace file for writing!");
            exit(EXIT_FAILURE);
        }

    if (fwrite(trace->buffer_ptr, __get_buffer_size(trace), 1, trace->ftrace) != 1) {
        perror("Flushing the buffer. Could not write measured data to the trace file!");
        exit(EXIT_FAILURE);
    }

    if (trace->allow_thread_safety)
        pthread_mutex_unlock(&trace->lock_evnt_flush);

    trace->buffer_cur = trace->buffer_ptr;
    trace->already_flushed = 1;
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(evnt_trace_t* trace, evnt_code_t code) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(0);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 0;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(0);
        evnt_flush_buffer(trace);
        evnt_probe0(trace, code);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(1);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 1;
        ((evnt_t *) cur_ptr)->param[0] = param1;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(1);
        evnt_flush_buffer(trace);
        evnt_probe1(trace, code, param1);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(2);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 2;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(2);
        evnt_flush_buffer(trace);
        evnt_probe2(trace, code, param1, param2);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(3);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 3;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(3);
        evnt_flush_buffer(trace);
        evnt_probe3(trace, code, param1, param2, param3);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(4);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 4;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(4);
        evnt_flush_buffer(trace);
        evnt_probe4(trace, code, param1, param2, param3, param4);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(5);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 5;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(5);
        evnt_flush_buffer(trace);
        evnt_probe5(trace, code, param1, param2, param3, param4, param5);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5, evnt_param_t param6) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(6);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 6;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(6);
        evnt_flush_buffer(trace);
        evnt_probe6(trace, code, param1, param2, param3, param4, param5, param6);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(7);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 7;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
        ((evnt_t *) cur_ptr)->param[6] = param7;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(7);
        evnt_flush_buffer(trace);
        evnt_probe7(trace, code, param1, param2, param3, param4, param5, param6, param7);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(8);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 8;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
        ((evnt_t *) cur_ptr)->param[6] = param7;
        ((evnt_t *) cur_ptr)->param[7] = param8;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(8);
        evnt_flush_buffer(trace);
        evnt_probe8(trace, code, param1, param2, param3, param4, param5, param6, param7, param8);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8,
        evnt_param_t param9) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(9);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 9;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
        ((evnt_t *) cur_ptr)->param[6] = param7;
        ((evnt_t *) cur_ptr)->param[7] = param8;
        ((evnt_t *) cur_ptr)->param[8] = param9;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(9);
        evnt_flush_buffer(trace);
        evnt_probe9(trace, code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event with ten arguments
 */
void evnt_probe10(evnt_trace_t* trace, evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3,
        evnt_param_t param4, evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8,
        evnt_param_t param9, evnt_param_t param10) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components(10);
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = evnt_get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 10;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
        ((evnt_t *) cur_ptr)->param[6] = param7;
        ((evnt_t *) cur_ptr)->param[7] = param8;
        ((evnt_t *) cur_ptr)->param[8] = param9;
        ((evnt_t *) cur_ptr)->param[9] = param10;
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur - get_event_components(10);
        evnt_flush_buffer(trace);
        evnt_probe10(trace, code, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function records an event in a raw state, where the size is the number of chars in the array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(evnt_trace_t* trace, evnt_code_t code, evnt_size_t size, evnt_data_t data[]) {
    if (!trace->evnt_initialized || trace->evnt_paused || trace->is_buffer_full)
        return;

    evnt_size_t i;
    evnt_buffer_t cur_ptr, next_ptr;

    // thread safety through atomic compare and swap method
    do {
        cur_ptr = trace->buffer_cur;
        next_ptr = trace->buffer_cur + get_event_components((evnt_size_t) ceil((double) size / sizeof(evnt_param_t)));
    } while (!evnt_cmpxchg(&trace->buffer_cur, cur_ptr, next_ptr));

    if (__get_buffer_size(trace) < trace->buffer_size) {
        ((evnt_raw_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_raw_t *) cur_ptr)->time = evnt_get_time();
        code = set_bit(code);
        ((evnt_raw_t *) cur_ptr)->code = code;
        ((evnt_raw_t *) cur_ptr)->size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                ((evnt_raw_t *) cur_ptr)->raw[i] = data[i];
    } else if (trace->allow_buffer_flush) {
        trace->buffer_cur = trace->buffer_cur
                - get_event_components((evnt_size_t) ceil((double) size / sizeof(evnt_param_t)));
        evnt_flush_buffer(trace);
        evnt_raw_probe(trace, code, size, data);
    } else
        // this applies only when the flushing is off
        trace->is_buffer_full = 1;
}

/*
 * This function finalizes the trace
 */
void evnt_fin_trace(evnt_trace_t* trace) {
    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    evnt_probe0(trace, EVNT_TRACE_END);

    evnt_flush_buffer(trace);

    fclose(trace->ftrace);
    free(trace->buffer_ptr);

    if (trace->allow_thread_safety)
        pthread_mutex_destroy(&trace->lock_evnt_flush);

    trace->ftrace = NULL;
    trace->buffer_ptr = NULL;
    free(trace->evnt_filename);
    trace->evnt_filename = NULL;
    trace->evnt_initialized = 0;
    trace->already_flushed = 0;
}
