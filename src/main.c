/*
 * Created by Roman Iakymchuk
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "evnt_types.h"
#include "evnt_write.h"
//#include "evnt_read.h"

int main(int argc, const char * argv[]) {

    char* filename = "trace.trace";
    buffer_flags buffer_flush = EVNT_BUFFER_FLUSH;
    thread_flags thread_safe = EVNT_NOTHREAD_SAFE;
    uint32_t buffer_size = 128;

    init_trace(filename, buffer_flush, thread_safe, buffer_size);

    finalize_trace();

    return EXIT_SUCCESS;
}
