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
#include <math.h>
#include <pthread.h>

#include "evnt_macro.h"
#include "evnt_write.h"

/*
 * To handle write conflicts while using pthread
 */
static pthread_mutex_t __evnt_flush_lock;

static evnt_trace_t __buffer_ptr;
static evnt_trace_t __buffer_cur;
static uint32_t __buffer_size = 512 * 1024; // 512KB is the optimal buffer size for recording events on Intel Core2
static buffer_flags_t __buffer_flush_flag = EVNT_BUFFER_FLUSH;
static thread_flags_t __thread_safe_flag = EVNT_NOTHREAD_SAFE;

static FILE* __ftrace;
static char* __evnt_filename;

static uint8_t __tid_activated = 0;

/*
 * __evnt_initialized is used to ensure that EZTrace does not start recording events before the initialization is finished
 */
static uint8_t __evnt_initialized = 0;

/*
 * __already_flushed is used to check whether the buffer was flushed as well as
 *                   to ensure that the correct and unique trace file was opened
 */
static uint8_t __already_flushed = 0;

/*
 * This function computes the size of data in buffer
 */
static uint32_t __get_buffer_size() {
    return sizeof(uint64_t) * ((evnt_trace_t) __buffer_cur - (evnt_trace_t) __buffer_ptr);
}

/*
 * This function returns the current time in ns
 */
static evnt_time_t __get_time() {
    // TODO: implement such function using rdtscll, which will return time in clock cycles
    struct timespec tp;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    return ((evnt_time_t) tp.tv_nsec + (evnt_time_t) (1000000000 * (tp.tv_sec)));
}

/*
 * Activate flushing buffer
 */
void enable_buffer_flush() {
    __buffer_flush_flag = EVNT_BUFFER_FLUSH;
}

/*
 * Deactivate flushing buffer. It is activated by default
 */
void disable_buffer_flush() {
    __buffer_flush_flag = EVNT_BUFFER_NOFLUSH;
}

/*
 * Activate thread-safety. It is not activated by default
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
 * Activate recording tid. It is not activated by default
 */
void enable_tid_record() {
    __tid_activated = 1;
}

/*
 * Deactivate recording tid
 */
void disable_tid_record() {
    __tid_activated = 0;
}

/*
 * Set a new name for the trace file
 */
void set_filename(char* filename) {
    if (__evnt_filename) {
        if (__already_flushed)
            fprintf(stderr,
                    "Warning: changing the trace file name to %s after some events have been saved in file %s\n",
                    filename, __evnt_filename);
        free(__evnt_filename);
    }

    // check whether the file name was set. If no, set it by default trace name.
    if (filename == NULL )
        sprintf(filename, "/tmp/%s_%s", getenv("USER"), "eztrace_log_rank_1");

    if (asprintf(&__evnt_filename, "%s", filename) == -1) {
        perror("Error: Cannot set the filename for recording events!\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * This function sets the buffer size
 */
static void __set_write_buffer_size(const uint32_t buf_size) {
    __buffer_size = buf_size;
}

/*
 * This function initializes the trace
 */
void init_trace(const uint32_t buf_size) {
    void *vp;

    __set_write_buffer_size(buf_size);

    // the size of the buffer is slightly bigger than it was required, because one additional event is added after the tracing
    vp = malloc(buf_size + get_event_size(EVNT_MAX_PARAMS));
    if (!vp) {
        perror("Could not allocate memory for the buffer!");
        exit(EXIT_FAILURE);
    }

    // set variables
    __buffer_ptr = vp;
    __buffer_cur = __buffer_ptr;

    // TODO: touch each block in buffer_ptr in order to load it

    if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        pthread_mutex_init(&__evnt_flush_lock, NULL );
    }

    __evnt_initialized = 1;
}

/*
 * This function finalizes the trace
 */
void fin_trace() {
    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    ((evnt_t *) __buffer_cur)->tid = CUR_TID;
    ((evnt_t *) __buffer_cur)->time = __get_time();
    ((evnt_t *) __buffer_cur)->code = EVNT_TRACE_END;
    ((evnt_t *) __buffer_cur)->nb_params = 0;

    __buffer_cur += get_event_components(0);

    flush_buffer();

    fclose(__ftrace);
    free(__buffer_ptr);

    __ftrace = NULL;
    __buffer_ptr = NULL;
    __evnt_filename = NULL;
    __evnt_initialized = 0;
    __already_flushed = 0;
}

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void flush_buffer() {
    if (!__evnt_initialized)
        return;

    pthread_mutex_lock(&__evnt_flush_lock);

    if (!__already_flushed)
        // check whether the trace file can be opened
        if (!(__ftrace = fopen(__evnt_filename, "w+"))) {
            perror("Could not open the trace file for writing!");
            exit(EXIT_FAILURE);
        }

    if (fwrite(__buffer_ptr, __get_buffer_size(), 1, __ftrace) != 1) {
        perror("Flushing the buffer. Could not write measured data to the trace file!");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&__evnt_flush_lock);

    __buffer_cur = __buffer_ptr;
    __already_flushed = 1;
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(evnt_code_t code) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 0;

        __buffer_cur += get_event_components(0);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe0(code);
    }
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(evnt_code_t code, evnt_param_t param1) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 1;
        ((evnt_t *) __buffer_cur)->param[0] = param1;

        __buffer_cur += get_event_components(1);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe1(code, param1);
    }
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(evnt_code_t code, evnt_param_t param1, evnt_param_t param2) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 2;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;

        __buffer_cur += get_event_components(2);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe2(code, param1, param2);
    }
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 3;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;

        __buffer_cur += get_event_components(3);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe3(code, param1, param2, param3);
    }
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 4;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;

        __buffer_cur += get_event_components(4);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe4(code, param1, param2, param3, param4);
    }
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 5;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;

        __buffer_cur += get_event_components(5);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe5(code, param1, param2, param3, param4, param5);
    }
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5, evnt_param_t param6) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 6;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;
        ((evnt_t *) __buffer_cur)->param[5] = param6;

        __buffer_cur += get_event_components(6);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe6(code, param1, param2, param3, param4, param5, param6);
    }
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5, evnt_param_t param6, evnt_param_t param7) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 7;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;
        ((evnt_t *) __buffer_cur)->param[5] = param6;
        ((evnt_t *) __buffer_cur)->param[6] = param7;

        __buffer_cur += get_event_components(7);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe7(code, param1, param2, param3, param4, param5, param6, param7);
    }
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 8;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;
        ((evnt_t *) __buffer_cur)->param[5] = param6;
        ((evnt_t *) __buffer_cur)->param[6] = param7;
        ((evnt_t *) __buffer_cur)->param[7] = param8;

        __buffer_cur += get_event_components(8);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe8(code, param1, param2, param3, param4, param5, param6, param7, param8);
    }
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8, evnt_param_t param9) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 9;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;
        ((evnt_t *) __buffer_cur)->param[5] = param6;
        ((evnt_t *) __buffer_cur)->param[6] = param7;
        ((evnt_t *) __buffer_cur)->param[7] = param8;
        ((evnt_t *) __buffer_cur)->param[8] = param9;

        __buffer_cur += get_event_components(9);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe9(code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }
}

/*
 * This function records an event with ten arguments
 */
void evnt_probe10(evnt_code_t code, evnt_param_t param1, evnt_param_t param2, evnt_param_t param3, evnt_param_t param4,
        evnt_param_t param5, evnt_param_t param6, evnt_param_t param7, evnt_param_t param8, evnt_param_t param9,
        evnt_param_t param10) {
    if (!__evnt_initialized)
        return;

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_t *) __buffer_cur)->time = __get_time();
        ((evnt_t *) __buffer_cur)->code = code;
        ((evnt_t *) __buffer_cur)->nb_params = 10;
        ((evnt_t *) __buffer_cur)->param[0] = param1;
        ((evnt_t *) __buffer_cur)->param[1] = param2;
        ((evnt_t *) __buffer_cur)->param[2] = param3;
        ((evnt_t *) __buffer_cur)->param[3] = param4;
        ((evnt_t *) __buffer_cur)->param[4] = param5;
        ((evnt_t *) __buffer_cur)->param[5] = param6;
        ((evnt_t *) __buffer_cur)->param[6] = param7;
        ((evnt_t *) __buffer_cur)->param[7] = param8;
        ((evnt_t *) __buffer_cur)->param[8] = param9;
        ((evnt_t *) __buffer_cur)->param[9] = param10;

        __buffer_cur += get_event_components(10);
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_probe10(code, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10);
    }
}

/*
 * This function records an event in a raw state, where the size is the number of chars in the array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(evnt_code_t code, evnt_size_t size, evnt_data_t data[]) {
    if (!__evnt_initialized)
        return;

    int i;
    if (__get_buffer_size() < __buffer_size) {
        ((evnt_raw_t *) __buffer_cur)->tid = CUR_TID;
        ((evnt_raw_t *) __buffer_cur)->time = __get_time();
        code = set_bit(code);
        ((evnt_raw_t *) __buffer_cur)->code = code;
        ((evnt_raw_t *) __buffer_cur)->size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                ((evnt_raw_t *) __buffer_cur)->raw[i] = data[i];

        __buffer_cur += 4 + (uint64_t) ceil((double) size / sizeof(uint64_t));
    } else if (__buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        // TODO: thread-safety
        flush_buffer();
        evnt_raw_probe(code, size, data);
    }
}
