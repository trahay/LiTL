/*
 * evnt_write.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "evnt_macro.h"
#include "evnt_write.h"

static trace __buffer_ptr;
static trace __buffer_cur;
static uint32_t __buffer_size = 256 * 1024; // 256 KB
static buffer_flags __buffer_flush_flag = EVNT_BUFFER_FLUSH;
static thread_flags __thread_safe_flag = EVNT_NOTHREAD_SAFE;

static FILE* __ftrace;
static char* __evnt_filename;

static int __is_flushed = 0;
int tid_activated = 0;

/*
 * This variable is used in order to make sure that EZTrace does nor start recording events before initialization finished
 */
static int __evnt_initialized = 0;

/*
 * This function computes the size of data in buffer
 */
static uint32_t __get_buffer_size() {
    return sizeof(uint64_t) * ((trace) __buffer_cur - (trace) __buffer_ptr);
}

/*
 * This function returns the current time in ns
 */
static uint64_t __get_time() {
    // TODO: implement such function using rdtscll, which will return time in clock cycles
    struct timespec tp;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    return ((uint64_t) tp.tv_nsec + (uint64_t) (1000000000 * (tp.tv_sec)));
}

/*
 * Activate flushing buffer. By default it is activated
 */
void enable_buffer_flush() {
    __buffer_flush_flag = EVNT_BUFFER_FLUSH;
}

/*
 * Deactivate flushing buffer
 */
void disable_buffer_flush() {
    __buffer_flush_flag = EVNT_BUFFER_NOFLUSH;
}

/*
 * Activate thread-safety. By default it is deactivated
 */
void enable_thread_safe() {
    __thread_safe_flag = EVNT_THREAD_SAFE;
}

/*
 * Deactivate thread-safety
 */
void disable_thread_safe() {
    __thread_safe_flag = EVNT_NOTHREAD_SAFE;
}

/*
 * Activate recording tid. By default it is not activated.
 */
void enable_tid_record() {
    tid_activated = 1;
}

/*
 * Deactivate recording tid
 */
void disable_tid_record() {
    tid_activated = 0;
}

/*
 * Set a new name for the trace file
 */
void set_filename(const char* filename) {
    if (__evnt_filename) {
        if (__is_flushed)
            fprintf(stderr, "Warning: change the trace file name to %s after some events have been saved in file %s\n",
                    filename, __evnt_filename);
        free(__evnt_filename);
    }
    if (asprintf(&__evnt_filename, "%s", filename) == -1)
        fprintf(stderr, "Error: Cannot set filename!\n");
}

/*
 * This function sets the buffer size
 */
void set_write_buffer_size(const uint32_t buf_size) {
    __buffer_size = buf_size;
}

/*
 * This function initializes the trace
 */
void init_trace(const char* filename, const uint32_t buf_size) {
    void *vp;

    // TODO: add a checker, so if any of the arguments is not given, then the default value will be taken

    /*// allocate memory in such way that it is aligned
    int ok;
    ok = posix_memalign(&vp, sizeof(uint64_t), buf_size + sizeof(evnt));
    if (ok != 0)
    perror("Could not allocate memory for the buffer!");*/
    // the size of buffer is slightly bigger than it was required, because one additional event is added after tracing
    vp = malloc(buf_size + sizeof(evnt));
    if (!vp)
        perror("Could not allocate memory for the buffer!");

    // set variables
    __buffer_ptr = vp;
    __buffer_cur = __buffer_ptr;
    __buffer_size = buf_size;
    set_write_buffer_size(buf_size);

    // TODO: perhaps, it is better to touch each block in buffer_ptr in order to load it

    // check whether the file name was set
    if (filename == NULL )
        // TODO: set the name dynamically using env variables
        set_filename("/tmp/roman_eztrace_log_rank_1");
    else
        set_filename(filename);

    // check whether the trace file can be opened
    if (!(__ftrace = fopen(filename, "w+"))) {
        perror("Could not open the trace file for writing!");
        exit(EXIT_FAILURE);
    }

    __evnt_initialized = 1;
}

/*
 * This function finalizes the trace
 */
void fin_trace() {
    __evnt_initialized = 0;

    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    ((evnt *) __buffer_cur)->tid = CUR_TID;
    ((evnt *) __buffer_cur)->time = __get_time();
    ((evnt *) __buffer_cur)->code = EVNT_TRACE_END;
    ((evnt *) __buffer_cur)->nb_args = 0;

    __buffer_cur += get_event_size(0);

    if (fwrite(__buffer_ptr, __get_buffer_size(), 1, __ftrace) != 1) {
        perror("Could not write measured data to the trace file!");
        exit(EXIT_FAILURE);
    }

    fclose(__ftrace);
    free(__buffer_ptr);
    __buffer_ptr = NULL;
}

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void flush_buffer() {
    if (!__evnt_initialized)
        return;

    if (fwrite(__buffer_ptr, __get_buffer_size(), 1, __ftrace) != 1)
        perror("Flushing the buffer. Could not write measured data to the trace file!");

    __buffer_cur = __buffer_ptr;
    __is_flushed = 1;
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(uint64_t code) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 0;

        __buffer_cur += get_event_size(0);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe0(code);
    }
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(uint64_t code, uint64_t param1) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 1;
        ((evnt *) __buffer_cur)->args[0] = param1;

        __buffer_cur += get_event_size(1);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe1(code, param1);
    }
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(uint64_t code, uint64_t param1, uint64_t param2) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 2;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;

        __buffer_cur += get_event_size(2);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe2(code, param1, param2);
    }
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 3;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;

        __buffer_cur += get_event_size(3);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe3(code, param1, param2, param3);
    }
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 4;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;

        __buffer_cur += get_event_size(4);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe4(code, param1, param2, param3, param4);
    }
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 5;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;
        ((evnt *) __buffer_cur)->args[4] = param5;

        __buffer_cur += get_event_size(5);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe5(code, param1, param2, param3, param4, param5);
    }
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 6;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;
        ((evnt *) __buffer_cur)->args[4] = param5;
        ((evnt *) __buffer_cur)->args[5] = param6;

        __buffer_cur += get_event_size(6);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe6(code, param1, param2, param3, param4, param5, param6);
    }
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 7;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;
        ((evnt *) __buffer_cur)->args[4] = param5;
        ((evnt *) __buffer_cur)->args[5] = param6;
        ((evnt *) __buffer_cur)->args[6] = param7;

        __buffer_cur += get_event_size(7);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe7(code, param1, param2, param3, param4, param5, param6, param7);
    }
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 8;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;
        ((evnt *) __buffer_cur)->args[4] = param5;
        ((evnt *) __buffer_cur)->args[5] = param6;
        ((evnt *) __buffer_cur)->args[6] = param7;
        ((evnt *) __buffer_cur)->args[7] = param8;

        __buffer_cur += get_event_size(8);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe8(code, param1, param2, param3, param4, param5, param6, param7, param8);
    }
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8, uint64_t param9) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt *) __buffer_cur)->tid = CUR_TID;
        ((evnt *) __buffer_cur)->time = __get_time();
        ((evnt *) __buffer_cur)->code = code;
        ((evnt *) __buffer_cur)->nb_args = 9;
        ((evnt *) __buffer_cur)->args[0] = param1;
        ((evnt *) __buffer_cur)->args[1] = param2;
        ((evnt *) __buffer_cur)->args[2] = param3;
        ((evnt *) __buffer_cur)->args[3] = param4;
        ((evnt *) __buffer_cur)->args[4] = param5;
        ((evnt *) __buffer_cur)->args[5] = param6;
        ((evnt *) __buffer_cur)->args[6] = param7;
        ((evnt *) __buffer_cur)->args[7] = param8;
        ((evnt *) __buffer_cur)->args[8] = param9;

        __buffer_cur += get_event_size(9);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe9(code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }
}

/*
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t code, uint64_t size, void* data) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_raw *) __buffer_cur)->tid = CUR_TID;
        ((evnt_raw *) __buffer_cur)->time = __get_time();
        ((evnt_raw *) __buffer_cur)->code = code;
        ((evnt_raw *) __buffer_cur)->size = size;
        if (size > 0)
            ((evnt_raw *) __buffer_cur)->data = data;

        __buffer_cur += size + 4 * sizeof(uint64_t);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_raw_probe(code, size, data);
    }
}
