/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © Télécom SudParis.
 * See COPYING in top-level directory.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "timer.h"
#include "evnt_macro.h"
#include "evnt_write.h"

/*
 * To handle write conflicts while using pthread
 */
static pthread_mutex_t __evnt_flush_lock;

static evnt_trace_t __buffer_ptr;
static evnt_trace_t __buffer_cur;
static uint32_t __buffer_size = 512 * 1024; // 512KB is the optimal buffer size for recording events on Intel Core2
// __buffer_flush indicates whether buffer flush is enabled (1) or not (0)
static uint8_t __buffer_flush = 1;
// __thread_safety indicates whether libevnt uses thread-safety (1) or not (0)
static uint8_t __thread_safety = 0;

static FILE* __ftrace;
static char* __evnt_filename;

static uint8_t __tid_activated = 0;

// __evnt_initialized is used to ensure that EZTrace does not start recording events before the initialization is finished
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
    return sizeof(evnt_trace_t) * ((evnt_trace_t) __buffer_cur - (evnt_trace_t) __buffer_ptr);
}

/*
 * Activate buffer flush
 */
void enable_buffer_flush() {
    __buffer_flush = 1;
}

/*
 * Deactivate buffer flush. It is activated by default
 */
void disable_buffer_flush() {
    __buffer_flush = 0;
}

/*
 * Activate thread-safety. It is not activated by default
 */
void enable_thread_safety() {
    __thread_safety = 1;
}

/*
 * Deactivate thread-safety
 */
void disable_thread_safetyty() {
    __thread_safety = 0;
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

    if (__thread_safety && __buffer_flush) {
        pthread_mutex_init(&__evnt_flush_lock, NULL );
    }

    __evnt_initialized = 1;
}

/*
 * This function finalizes the trace
 */
void fin_trace() {
    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    evnt_probe0(EVNT_TRACE_END);

    flush_buffer();

    fclose(__ftrace);
    free(__buffer_ptr);

    if (__thread_safety)
        pthread_mutex_destroy(&__evnt_flush_lock);

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

    if (__thread_safety)
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

    if (__thread_safety)
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(0);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 0;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(1);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 1;
        ((evnt_t *) cur_ptr)->param[0] = param1;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(2);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 2;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(3);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 3;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(4);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 4;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(5);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 5;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(6);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 6;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(7);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
        ((evnt_t *) cur_ptr)->code = code;
        ((evnt_t *) cur_ptr)->nb_params = 7;
        ((evnt_t *) cur_ptr)->param[0] = param1;
        ((evnt_t *) cur_ptr)->param[1] = param2;
        ((evnt_t *) cur_ptr)->param[2] = param3;
        ((evnt_t *) cur_ptr)->param[3] = param4;
        ((evnt_t *) cur_ptr)->param[4] = param5;
        ((evnt_t *) cur_ptr)->param[5] = param6;
        ((evnt_t *) cur_ptr)->param[6] = param7;
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(8);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
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
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(9);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
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
    } else if (__buffer_flush) {
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

    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(10);
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_t *) cur_ptr)->time = get_time();
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
    } else if (__buffer_flush) {
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
    evnt_trace_t cur_ptr;

    if (__thread_safety)
        pthread_mutex_lock(&__evnt_flush_lock);
    cur_ptr = __buffer_cur;
    __buffer_cur += get_event_components(ceil((double) size / sizeof(evnt_param_t)));
    if (__thread_safety)
        pthread_mutex_unlock(&__evnt_flush_lock);

    if (__get_buffer_size() < __buffer_size) {
        ((evnt_raw_t *) cur_ptr)->tid = CUR_TID;
        ((evnt_raw_t *) cur_ptr)->time = get_time();
        code = set_bit(code);
        ((evnt_raw_t *) cur_ptr)->code = code;
        ((evnt_raw_t *) cur_ptr)->size = size;
        if (size > 0)
            for (i = 0; i < size; i++)
                ((evnt_raw_t *) cur_ptr)->raw[i] = data[i];
    } else if (__buffer_flush) {
        flush_buffer();
        evnt_raw_probe(code, size, data);
    }
}
