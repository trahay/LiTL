/*
 * evnt_write.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdlib.h>
#include <time.h>

#include "evnt_macro.h"
#include "evnt_write.h"

static trace buffer_ptr;
trace buffer_cur;

static uint32_t buffer_size = 32 * 1024; // 32 KB
static buffer_flags buffer_flush_flag = EVNT_BUFFER_FLUSH;
static thread_flags thread_safe_flag = EVNT_NOTHREAD_SAFE;
static FILE* _ftrace;

/*
 * This function computes the size of data in buffer
 */
static uint32_t _get_buffer_size() {
    return sizeof(uint64_t) * ((trace) buffer_cur - (trace) buffer_ptr);
}

/*
 * This function sets the buffer size
 */
void set_write_buffer_size(const uint32_t buf_size) {
    buffer_size = buf_size;
}

/*
 * This function returns the current time in ns
 */
static uint64_t _get_time() {
    // TODO: implement such function using rdtscll, which will return time in clock cycles
    struct timespec tp;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    return ((uint64_t) tp.tv_nsec + (uint64_t) (1000000000 * (tp.tv_sec)));
}

/*
 * This function sets the buffer flush flag
 */
void set_buffer_flag(const buffer_flags buffer_flag) {
    buffer_flush_flag = buffer_flag;
}

/*
 * This function sets the thread safe flag
 */
void set_thread_flag(const thread_flags thread_flag) {
    thread_safe_flag = thread_flag;
}

/*
 * This function initializes the trace
 */
void init_trace(const char* file_path, const buffer_flags buffer_flag, const thread_flags thread_flag,
        const uint32_t buf_size) {
    int ok;
    void *vp;

    // TODO: add a checker, so if any of the arguments is not given, then the default value will be taken

    // allocate memory in such way that it is aligned
    // the size of buffer is slightly bigger than it was required, because one additional event is added after tracing
    ok = posix_memalign(&vp, sizeof(uint64_t), buf_size + sizeof(evnt));
    if (ok != 0)
        perror("Could not allocate memory for the buffer!");

    // set variables
    buffer_ptr = vp;
    buffer_cur = buffer_ptr;
    buffer_size = buf_size;
    buffer_flush_flag = buffer_flag;
    thread_safe_flag = thread_flag;

    set_write_buffer_size(buf_size);
    set_buffer_flag(buffer_flag);
    set_thread_flag(thread_flag);

    // TODO: perhaps, it is better to touch each block in buffer_ptr in order to load it

    // check whether the trace file can be opened
    if (!(_ftrace = fopen(file_path, "w+")))
        perror("Could not open the trace file for writing!");
}

/*
 * This function finalizes the trace
 */
void fin_trace() {
    // write an event with the EVNT_TRACE_END (= 0) code in order to indicate the end of tracing
    ((evnt *) buffer_cur)->tid = TID_CUR;
    ((evnt *) buffer_cur)->time = _get_time();
    ((evnt *) buffer_cur)->code = EVNT_TRACE_END;
    ((evnt *) buffer_cur)->nb_args = 0;

    buffer_cur += get_event_size(0);

    if (fwrite(buffer_ptr, _get_buffer_size(), 1, _ftrace) != 1)
        perror("Could not write measured data to the trace file!");

    fclose(_ftrace);
    free(buffer_ptr);
}

/*
 * This function writes the recorded events from the buffer to the trace file
 */
void flush_buffer() {
    if (fwrite(buffer_ptr, _get_buffer_size(), 1, _ftrace) != 1)
        perror("Flushing the buffer. Could not write measured data to the trace file!");

    buffer_cur = buffer_ptr;
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(uint64_t code) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 0;

        buffer_cur += get_event_size(0);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe0(code);
    }
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(uint64_t code, uint64_t param1) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 1;
        ((evnt *) buffer_cur)->args[0] = param1;

        buffer_cur += get_event_size(1);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe1(code, param1);
    }
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(uint64_t code, uint64_t param1, uint64_t param2) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 2;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;

        buffer_cur += get_event_size(2);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe2(code, param1, param2);
    }
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 3;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;

        buffer_cur += get_event_size(3);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe3(code, param1, param2, param3);
    }
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 4;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;

        buffer_cur += get_event_size(4);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe4(code, param1, param2, param3, param4);
    }
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 5;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;
        ((evnt *) buffer_cur)->args[4] = param5;

        buffer_cur += get_event_size(5);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe5(code, param1, param2, param3, param4, param5);
    }
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 6;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;
        ((evnt *) buffer_cur)->args[4] = param5;
        ((evnt *) buffer_cur)->args[5] = param6;

        buffer_cur += get_event_size(6);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe6(code, param1, param2, param3, param4, param5, param6);
    }
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 7;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;
        ((evnt *) buffer_cur)->args[4] = param5;
        ((evnt *) buffer_cur)->args[5] = param6;
        ((evnt *) buffer_cur)->args[6] = param7;

        buffer_cur += get_event_size(7);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe7(code, param1, param2, param3, param4, param5, param6, param7);
    }
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 8;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;
        ((evnt *) buffer_cur)->args[4] = param5;
        ((evnt *) buffer_cur)->args[5] = param6;
        ((evnt *) buffer_cur)->args[6] = param7;
        ((evnt *) buffer_cur)->args[7] = param8;

        buffer_cur += get_event_size(8);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe8(code, param1, param2, param3, param4, param5, param6, param7, param8);
    }
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8, uint64_t param9) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt *) buffer_cur)->tid = TID_CUR;
        ((evnt *) buffer_cur)->time = _get_time();
        ((evnt *) buffer_cur)->code = code;
        ((evnt *) buffer_cur)->nb_args = 9;
        ((evnt *) buffer_cur)->args[0] = param1;
        ((evnt *) buffer_cur)->args[1] = param2;
        ((evnt *) buffer_cur)->args[2] = param3;
        ((evnt *) buffer_cur)->args[3] = param4;
        ((evnt *) buffer_cur)->args[4] = param5;
        ((evnt *) buffer_cur)->args[5] = param6;
        ((evnt *) buffer_cur)->args[6] = param7;
        ((evnt *) buffer_cur)->args[7] = param8;
        ((evnt *) buffer_cur)->args[8] = param9;

        buffer_cur += get_event_size(9);
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_probe9(code, param1, param2, param3, param4, param5, param6, param7, param8, param9);
    }
}

/*
 * This function records an event in a raw state, where the size is #args in the void* array
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t code, uint64_t size, void* data) {
    if (_get_buffer_size() < buffer_size) {
        ((evnt_raw *) buffer_cur)->tid = TID_CUR;
        ((evnt_raw *) buffer_cur)->time = _get_time();
        ((evnt_raw *) buffer_cur)->code = code;
        ((evnt_raw *) buffer_cur)->size = size;
        ((evnt_raw *) buffer_cur)->data = data;

        buffer_cur += get_event_size(size / sizeof(uint64_t));
    } else if (buffer_flush_flag == EVNT_BUFFER_FLUSH) {
        flush_buffer();
        evnt_raw_probe(code, size, data);
    }
}
