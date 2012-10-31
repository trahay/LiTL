/*
 * evnt_write.c
 *
 *  Created on: Oct 31, 2012
 *      Author: Roman Iakymchuk
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "evnt_write.h"

static uint64_t* buffer_ptr;
//uint64_t* buffer_cur;
static uint32_t buffer_size = 32 * 1024; // MB
static buffer_flags buffer_flush;
static thread_flags thread_safe;
static FILE* ftrace;

static uint32_t event_size(uint32_t nb_args) {
    assert(nb_args >= 0);

    return sizeof(uint64_t) * (nb_args + 4);
}

/*
 * This function initializes the trace
 */
void init_trace(char* file_path, buffer_flags flush_flag, thread_flags thread_flag, uint32_t buf_size) {
    int ok;
    void *vp;

    // buffer should keep at least one event
    assert(buf_size > event_size(EVNT_MAX_PARAMS));
    //sizeof(evnt)

    // allocate memory to be sure that it is aligned
    ok = posix_memalign(&vp, sizeof(uint64_t), buf_size);
    if (ok != 0)
        perror("Could not allocate memory for the buffer!");
    buffer_ptr = vp;

    // set variables
    //    buffer_cur = buffer_ptr;
    buffer_size = buf_size;
    buffer_flush = flush_flag;
    thread_safe = thread_flag;

    // check whether the trace file can be opened
    if (!(ftrace = fopen(file_path, "w+")))
        perror("Could not open the trace file!");
}

/*
 * This function finalizes the trace
 */
void finalize_trace() {
}

/*
 * This function records an event without any arguments
 */
void evnt_probe0(uint64_t code) {
}

/*
 * This function records an event with one argument
 */
void evnt_probe1(uint64_t code, uint64_t param1) {
}

/*
 * This function records an event with two arguments
 */
void evnt_probe2(uint64_t code, uint64_t param1, uint64_t param2) {
}

/*
 * This function records an event with three arguments
 */
void evnt_probe3(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3) {
}

/*
 * This function records an event with four arguments
 */
void evnt_probe4(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4) {
}

/*
 * This function records an event with five arguments
 */
void evnt_probe5(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5) {
}

/*
 * This function records an event with six arguments
 */
void evnt_probe6(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6) {
}

/*
 * This function records an event with seven arguments
 */
void evnt_probe7(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7) {
}

/*
 * This function records an event with eight arguments
 */
void evnt_probe8(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8) {
}

/*
 * This function records an event with nine arguments
 */
void evnt_probe9(uint64_t code, uint64_t param1, uint64_t param2, uint64_t param3, uint64_t param4, uint64_t param5,
        uint64_t param6, uint64_t param7, uint64_t param8, uint64_t param9) {
}

/*
 * This function records an event in a raw state.
 * That helps to discover places where the application has crashed while using EZTrace
 */
void evnt_raw_probe(uint64_t code, uint64_t size, void* data) {
}

/*
 * This function writes the recorded events from the buffer to the file
 */
void _buffer_flush() {
}
